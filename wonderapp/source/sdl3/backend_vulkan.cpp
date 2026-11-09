/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <vulkan/vulkan.h>

#include <sdlwindow.h>

#include <SDL3/SDL_vulkan.h>

#include <wax_vulkanbackend.h>
#include <wax_vulkancontext.h>

#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>


using namespace wg;
using namespace wax;

//____ SDLWindowVulkan ________________________________________________________
//
// Wax renders into an image of our own, the default canvas, which keeps what was
// rendered between frames, as only the dirty rects are drawn. Each frame that
// rendered anything is copied to an image of the swap chain and presented, since
// those don't keep their content.
//
// Environment variables:
//
//   WAPP_VK_DEVICE=n|name		Picks the physical device by index or part of its name.
//   WAPP_VK_VALIDATION=1		Turns the Khronos validation layer on, if installed.
//								On by default in debug builds, 0 turns it off.

class SDLWindowVulkan : public SDLWindow
{
public:
	SDLWindowVulkan(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowVulkan();

	void	render() override;
	void 	onWindowSizeUpdated() override;

	void	releaseVulkanObjects();

private:

	bool	_createSwapchain();
	void	_destroySwapchain();
	bool	_createCanvas();
	void	_destroyCanvas();
	void	_setDefaultCanvas();
	void	_present();

	VkSurfaceKHR				m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR				m_swapchain = VK_NULL_HANDLE;
	VkFormat					m_swapchainFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D					m_extent = { 0, 0 };
	std::vector<VkImage>		m_swapchainImages;
	std::vector<VkSemaphore>	m_renderDone;			// One per swap chain image, signalled for presenting it.
	bool						m_bSwapchainStale = false;

	// Our canvas, which Wax renders into and which is copied to the swap chain.

	VulkanContext::Image		m_canvas;
	VkImageView					m_canvasView = VK_NULL_HANDLE;
	VkFormat					m_canvasFormat = VK_FORMAT_UNDEFINED;

	// Copying and presenting, two frames in flight.

	static const int			c_nbFrames = 2;

	VkCommandPool				m_pool = VK_NULL_HANDLE;
	VkCommandBuffer				m_cmd[c_nbFrames] = {};
	VkSemaphore					m_imageAcquired[c_nbFrames] = {};
	uint64_t					m_submitted[c_nbFrames] = {};
	int							m_frame = 0;

	bool						m_bHidden = false;
};


static VkInstance						g_instance = VK_NULL_HANDLE;
static VkPhysicalDevice					g_physicalDevice = VK_NULL_HANDLE;
static VkDevice							g_device = VK_NULL_HANDLE;
static VkQueue							g_queue = VK_NULL_HANDLE;
static uint32_t							g_queueFamily = 0;
static VkDebugUtilsMessengerEXT			g_messenger = VK_NULL_HANDLE;

static VulkanBackend_p					g_pBackend;
static std::vector<SDLWindowVulkan*>	g_windows;

//____ _debugCallback() _______________________________________________________

static VKAPI_ATTR VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
													const VkDebugUtilsMessengerCallbackDataEXT* pData, void* pUserData)
{
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		fprintf(stderr, "Vulkan %s: %s\n", severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ? "error" : "warning", pData->pMessage);

	return VK_FALSE;
}

//____ _wantValidation() ______________________________________________________

static bool _wantValidation()
{
	const char * pValue = SDL_getenv("WAPP_VK_VALIDATION");

	if (pValue)
		return pValue[0] != '0';

#ifdef NDEBUG
	return false;
#else
	return true;
#endif
}

//____ _hasLayer() ____________________________________________________________

static bool _hasLayer(const char * pName)
{
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	std::vector<VkLayerProperties> layers(count);
	vkEnumerateInstanceLayerProperties(&count, layers.data());

	for (auto& layer : layers)
		if (strcmp(layer.layerName, pName) == 0)
			return true;

	return false;
}

//____ _deviceIsUsable() ______________________________________________________
//
// Vulkan 1.3, the extensions and features VulkanBackend needs, swap chains and a
// queue that draws and presents.

static bool _deviceIsUsable(VkPhysicalDevice device, uint32_t& queueFamily)
{
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);

	if (props.apiVersion < VK_API_VERSION_1_3)
		return false;

	uint32_t nExtensions = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &nExtensions, nullptr);
	std::vector<VkExtensionProperties> extensions(nExtensions);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &nExtensions, extensions.data());

	auto hasExtension = [&](const char * pName)
	{
		for (auto& ext : extensions)
			if (strcmp(ext.extensionName, pName) == 0)
				return true;
		return false;
	};

	int nRequired = 0;
	auto pRequired = VulkanBackend::requiredDeviceExtensions(nRequired);

	for (int i = 0; i < nRequired; i++)
		if (!hasExtension(pRequired[i]))
			return false;

	if (!hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
		return false;

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);

	if (!features.dualSrcBlend)
		return false;

	uint32_t nFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &nFamilies, nullptr);
	std::vector<VkQueueFamilyProperties> families(nFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &nFamilies, families.data());

	for (uint32_t i = 0; i < nFamilies; i++)
	{
		if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && SDL_Vulkan_GetPresentationSupport(g_instance, device, i))
		{
			queueFamily = i;
			return true;
		}
	}

	return false;
}

//____ _pickPhysicalDevice() __________________________________________________

static bool _pickPhysicalDevice()
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(g_instance, &count, nullptr);

	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(g_instance, &count, devices.data());

	const char * pWanted = SDL_getenv("WAPP_VK_DEVICE");

	int bestScore = -1;

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t family;
		if (!_deviceIsUsable(devices[i], family))
			continue;

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(devices[i], &props);

		int score;

		if (pWanted)
		{
			bool bIndex = pWanted[0] >= '0' && pWanted[0] <= '9';
			bool bMatch = bIndex ? (uint32_t(atoi(pWanted)) == i) : (strstr(props.deviceName, pWanted) != nullptr);
			score = bMatch ? 100 : 0;
		}
		else
		{
			switch (props.deviceType)
			{
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:		score = 4; break;
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:	score = 3; break;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:		score = 2; break;
				default:										score = 1; break;
			}
		}

		if (score > bestScore)
		{
			bestScore = score;
			g_physicalDevice = devices[i];
			g_queueFamily = family;
		}
	}

	if (!g_physicalDevice)
		return false;

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(g_physicalDevice, &props);
	printf("wappsdl3vk: Rendering with %s\n", props.deviceName);
	return true;
}

//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	if (!SDL_Vulkan_LoadLibrary(nullptr))
	{
		printf("Unable to load Vulkan: %s\n", SDL_GetError());
		return false;
	}

	// Instance, with what SDL needs for window surfaces.

	Uint32 nSDLExtensions = 0;
	char const * const * pSDLExtensions = SDL_Vulkan_GetInstanceExtensions(&nSDLExtensions);

	std::vector<const char*> extensions(pSDLExtensions, pSDLExtensions + nSDLExtensions);
	std::vector<const char*> layers;

	bool bValidation = _wantValidation() && _hasLayer("VK_LAYER_KHRONOS_validation");

	if (bValidation)
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = "WonderApp";
	appInfo.pEngineName = "Wax";
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = uint32_t(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();
	instanceInfo.enabledLayerCount = uint32_t(layers.size());
	instanceInfo.ppEnabledLayerNames = layers.data();

	if (vkCreateInstance(&instanceInfo, nullptr, &g_instance) != VK_SUCCESS)
	{
		printf("Unable to create Vulkan 1.3 instance.\n");
		return false;
	}

	if (bValidation)
	{
		auto create = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(g_instance, "vkCreateDebugUtilsMessengerEXT");

		VkDebugUtilsMessengerCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = _debugCallback;

		if (create)
			create(g_instance, &info, nullptr, &g_messenger);
	}

	if (!_pickPhysicalDevice())
	{
		printf("No Vulkan 1.3 device with what VulkanBackend needs.\n");
		return false;
	}

	// Device, with what VulkanBackend needs, see VulkanBackend.

	float priority = 1.f;

	VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = g_queueFamily;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &priority;

	VkPhysicalDeviceVulkan13Features features13 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features13.dynamicRendering = VK_TRUE;
	features13.synchronization2 = VK_TRUE;
	features13.shaderDemoteToHelperInvocation = VK_TRUE;		// discard, as DXC compiles it.

	VkPhysicalDeviceVulkan12Features features12 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.pNext = &features13;
	features12.timelineSemaphore = VK_TRUE;

	VkPhysicalDeviceFeatures2 features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	features.pNext = &features12;
	features.features.dualSrcBlend = VK_TRUE;

	int nRequired = 0;
	auto pRequired = VulkanBackend::requiredDeviceExtensions(nRequired);

	std::vector<const char*> deviceExtensions(pRequired, pRequired + nRequired);
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceInfo.pNext = &features;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = uint32_t(deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(g_physicalDevice, &deviceInfo, nullptr, &g_device) != VK_SUCCESS)
	{
		printf("Unable to create Vulkan device.\n");
		return false;
	}

	vkGetDeviceQueue(g_device, g_queueFamily, 0, &g_queue);

	if (!VulkanBackend::setDevice(g_instance, g_physicalDevice, g_device, g_queue, g_queueFamily))
		return false;

	g_pBackend = VulkanBackend::create();
	if (!g_pBackend)
		return false;

	auto pDevice = GfxDeviceGen2::create(g_pBackend);
	wg::Base::setDefaultDevice(pDevice);

	wg::Base::setDefaultSurfaceFactory(g_pBackend->surfaceFactory());
	wg::Base::setDefaultEdgemapFactory(g_pBackend->edgemapFactory());
	wg::Base::setDefaultWarpmapFactory(g_pBackend->warpmapFactory());

	return true;
}

//____ backend_specific_exit() ________________________________________________
//
// Surfaces and such of the app may outlive us, in which case they are left for
// the process to clean up, and so is the device.

void backend_specific_exit()
{
	VulkanContext::waitIdle();

	for (auto pWin : g_windows)
		pWin->releaseVulkanObjects();

	g_pBackend = nullptr;

	if (VulkanContext::nbBackends() > 0)
		return;

	VulkanBackend::setDevice(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 0);

	vkDestroyDevice(g_device, nullptr);
	g_device = VK_NULL_HANDLE;

	if (g_messenger)
	{
		auto destroy = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(g_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroy)
			destroy(g_instance, g_messenger, nullptr);
		g_messenger = VK_NULL_HANDLE;
	}

	vkDestroyInstance(g_instance, nullptr);
	g_instance = VK_NULL_HANDLE;
}

//____ backend_specific_update() ______________________________________________

void backend_specific_update()
{
}

//____ create_backend_specific_window() ________________________________________

SDLWindow * create_backend_specific_window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new SDLWindowVulkan(pUserWindow, origin, pos, size, title, resizable, open);
}

//____ SDLWindowVulkan() ______________________________________________________

SDLWindowVulkan::SDLWindowVulkan(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	g_windows.push_back(this);

	if (!_createSDLWindow(title, origin, pos, size, SDL_WINDOW_VULKAN, resizable, open))
		return;

	if (!SDL_Vulkan_CreateSurface(m_pSDLWindow, g_instance, nullptr, &m_surface))
	{
		printf("Unable to create Vulkan surface for window: %s\n", SDL_GetError());
		return;
	}

	VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = g_queueFamily;
	vkCreateCommandPool(g_device, &poolInfo, nullptr, &m_pool);

	VkCommandBufferAllocateInfo alloc = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	alloc.commandPool = m_pool;
	alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc.commandBufferCount = c_nbFrames;
	vkAllocateCommandBuffers(g_device, &alloc, m_cmd);

	VkSemaphoreCreateInfo semInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	for (int i = 0; i < c_nbFrames; i++)
		vkCreateSemaphore(g_device, &semInfo, nullptr, &m_imageAcquired[i]);

	_createSwapchain();
	_createCanvas();
	_setDefaultCanvas();

	m_pRootPanel = RootPanel::create(CanvasRef::Default, wg::Base::defaultDevice());

	//TODO: This is ugly. It should be handled when windows gets focused.

	wg::Base::inputHandler()->setFocusedWindow(m_pRootPanel);
}

//____ ~SDLWindowVulkan() _____________________________________________________

SDLWindowVulkan::~SDLWindowVulkan()
{
	g_windows.erase(std::remove(g_windows.begin(), g_windows.end(), this), g_windows.end());
	releaseVulkanObjects();
}

//____ releaseVulkanObjects() _________________________________________________

void SDLWindowVulkan::releaseVulkanObjects()
{
	if (!g_device)
		return;

	VulkanContext::waitIdle();

	_destroyCanvas();
	_destroySwapchain();

	for (auto& semaphore : m_imageAcquired)
	{
		if (semaphore)
			vkDestroySemaphore(g_device, semaphore, nullptr);
		semaphore = VK_NULL_HANDLE;
	}

	if (m_pool)
		vkDestroyCommandPool(g_device, m_pool, nullptr);
	m_pool = VK_NULL_HANDLE;

	if (m_surface)
		SDL_Vulkan_DestroySurface(g_instance, m_surface, nullptr);
	m_surface = VK_NULL_HANDLE;
}

//____ _createSwapchain() _____________________________________________________

bool SDLWindowVulkan::_createSwapchain()
{
	if (!m_surface)
		return false;

	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physicalDevice, m_surface, &caps);

	SizeI pixels = _sizeInPixels();

	VkExtent2D extent = caps.currentExtent;
	if (extent.width == 0xFFFFFFFF)
		extent = { uint32_t(pixels.w), uint32_t(pixels.h) };

	extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
	extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);

	if (extent.width == 0 || extent.height == 0)
	{
		m_bHidden = true;
		return false;
	}

	// BGRA, so that our canvas can be copied as it is. Its sRGB format has
	// encoded the pixels already, so a swap chain without one gets them right too.

	uint32_t nFormats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(g_physicalDevice, m_surface, &nFormats, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(nFormats);
	vkGetPhysicalDeviceSurfaceFormatsKHR(g_physicalDevice, m_surface, &nFormats, formats.data());

	VkSurfaceFormatKHR chosen = formats.empty() ? VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } : formats[0];

	for (auto& format : formats)
	{
		if ((format.format == VK_FORMAT_B8G8R8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_UNORM) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			chosen = format;
			break;
		}
	}

	uint32_t nImages = std::max(caps.minImageCount, 2u);
	if (caps.maxImageCount > 0)
		nImages = std::min(nImages, caps.maxImageCount);

	VkSwapchainCreateInfoKHR info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	info.surface = m_surface;
	info.minImageCount = nImages;
	info.imageFormat = chosen.format;
	info.imageColorSpace = chosen.colorSpace;
	info.imageExtent = extent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.preTransform = caps.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	info.clipped = VK_TRUE;
	info.oldSwapchain = m_swapchain;

	if (!(caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR))
		info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkResult result = vkCreateSwapchainKHR(g_device, &info, nullptr, &swapchain);

	_destroySwapchain();

	if (result != VK_SUCCESS)
	{
		printf("vkCreateSwapchainKHR failed: %d\n", int(result));
		return false;
	}

	m_swapchain = swapchain;
	m_swapchainFormat = chosen.format;
	m_extent = extent;

	uint32_t count = 0;
	vkGetSwapchainImagesKHR(g_device, m_swapchain, &count, nullptr);
	m_swapchainImages.resize(count);
	vkGetSwapchainImagesKHR(g_device, m_swapchain, &count, m_swapchainImages.data());

	VkSemaphoreCreateInfo semInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	m_renderDone.resize(count);
	for (auto& semaphore : m_renderDone)
		vkCreateSemaphore(g_device, &semInfo, nullptr, &semaphore);

	m_bSwapchainStale = false;
	m_bHidden = false;
	return true;
}

//____ _destroySwapchain() ____________________________________________________

void SDLWindowVulkan::_destroySwapchain()
{
	for (auto semaphore : m_renderDone)
		vkDestroySemaphore(g_device, semaphore, nullptr);

	m_renderDone.clear();
	m_swapchainImages.clear();

	if (m_swapchain)
		vkDestroySwapchainKHR(g_device, m_swapchain, nullptr);

	m_swapchain = VK_NULL_HANDLE;
}

//____ _createCanvas() ________________________________________________________
//
// Same size as the swap chain and the sRGB format of its channel order. Cleared,
// and left in TRANSFER_SRC_OPTIMAL, where it rests between Wax's sessions.

bool SDLWindowVulkan::_createCanvas()
{
	if (m_extent.width == 0)
		return false;

	bool bRGBA = (m_swapchainFormat == VK_FORMAT_R8G8B8A8_UNORM || m_swapchainFormat == VK_FORMAT_R8G8B8A8_SRGB);
	m_canvasFormat = bRGBA ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_B8G8R8A8_SRGB;

	VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = m_canvasFormat;
	info.extent = { m_extent.width, m_extent.height, 1 };
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (!VulkanContext::createImage(info, m_canvas))
		return false;

	VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.image = m_canvas.image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = m_canvasFormat;
	viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	vkCreateImageView(g_device, &viewInfo, nullptr, &m_canvasView);

	VkCommandBuffer cmd = VulkanContext::beginOneShot();
	if (cmd)
	{
		VulkanContext::transition(cmd, m_canvas.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkClearColorValue black = { { 0.f, 0.f, 0.f, 1.f } };
		VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdClearColorImage(cmd, m_canvas.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &black, 1, &range);

		VulkanContext::transition(cmd, m_canvas.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanContext::endOneShot(cmd);
	}

	return true;
}

//____ _destroyCanvas() _______________________________________________________

void SDLWindowVulkan::_destroyCanvas()
{
	VulkanContext::destroyLater(m_canvasView);
	VulkanContext::destroyLater(m_canvas);
}

//____ _setDefaultCanvas() ____________________________________________________

void SDLWindowVulkan::_setDefaultCanvas()
{
	g_pBackend->setDefaultCanvas(m_canvas.image, m_canvasView, m_canvasFormat, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
								 { spx(m_extent.width * 64), spx(m_extent.height * 64) }, m_pRootPanel ? m_pRootPanel->scale() : _scale());
}

//____ render() _______________________________________________________________

void SDLWindowVulkan::render()
{
	if (m_bSwapchainStale && !m_bHidden)
	{
		onWindowSizeUpdated();			// Renders it all again.
		return;
	}

	if (m_bHidden || !m_pRootPanel || !m_canvas.image || m_pRootPanel->nbDirtyRects() == 0)
		return;

	_setDefaultCanvas();
	m_pRootPanel->render();

	_present();
}

//____ _present() _____________________________________________________________
//
// Copies our canvas to the next image of the swap chain and presents it.

void SDLWindowVulkan::_present()
{
	if (!m_swapchain || VulkanContext::isDeviceLost())
		return;

	m_frame = (m_frame + 1) % c_nbFrames;

	// The command buffer and semaphore of this frame must be done with.

	VulkanContext::wait(m_submitted[m_frame]);

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(g_device, m_swapchain, UINT64_MAX, m_imageAcquired[m_frame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		m_bSwapchainStale = true;
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		return;

	VkCommandBuffer cmd = m_cmd[m_frame];
	vkResetCommandBuffer(cmd, 0);

	VkCommandBufferBeginInfo begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd, &begin);

	VkImage target = m_swapchainImages[imageIndex];

	VulkanContext::transition(cmd, target, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Waits for Wax's rendering into the canvas, which came before us in the queue.

	VkMemoryBarrier2 barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;

	VkDependencyInfo dep = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	dep.memoryBarrierCount = 1;
	dep.pMemoryBarriers = &barrier;
	vkCmdPipelineBarrier2(cmd, &dep);

	VkImageCopy region = {};
	region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.extent = { m_extent.width, m_extent.height, 1 };

	vkCmdCopyImage(cmd, m_canvas.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VulkanContext::transition(cmd, target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	vkEndCommandBuffer(cmd);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	m_submitted[m_frame] = VulkanContext::submit(cmd, 1, &m_imageAcquired[m_frame], &waitStage, 1, &m_renderDone[imageIndex]);

	VkPresentInfoKHR present = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	present.waitSemaphoreCount = 1;
	present.pWaitSemaphores = &m_renderDone[imageIndex];
	present.swapchainCount = 1;
	present.pSwapchains = &m_swapchain;
	present.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(g_queue, &present);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		m_bSwapchainStale = true;

	VulkanContext::collectGarbage();
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowVulkan::onWindowSizeUpdated()
{
	if (!m_pRootPanel || !m_surface)
		return;

	VulkanContext::waitIdle();

	_destroyCanvas();

	if (!_createSwapchain() || !_createCanvas())
		return;							// Minimized, we wait for a real size.

	_setDefaultCanvas();
	m_pRootPanel->setCanvas(CanvasRef::Default);

	_notifyUserWindowOfResize();
	render();
}
