

#include <wondergui.h>

#include <wonderapp.h>

#include <wg_softkernels_default.h>
#include <wg_softedgemapfactory.h>
#include <wg_softsurfacefactory.h>



#include "device.h"
#include "testsuites/testsuite.h"



class GfxDeviceTester : public WonderApp
{
	friend class WonderApp;
public:


	bool	init(wapp::API* pAPI) override;
	bool	update() override;
	void	exit() override;

	void	closeWindow(wapp::Window* pWindow) override;


	// Pre-init configuration

	void	addTestDevice(Device_p pDevice);
	void	destroy_testdevices();



	
protected:
	GfxDeviceTester();
	~GfxDeviceTester();

	enum class DisplayMode
	{
		Testee,
		Reference,
		Both,
		Diff,
		Time,
		Compare
	};

	enum DeviceEnum
	{
		REFERENCE = 0,
		TESTEE = 1,
	};

	enum class ClipList
	{
		One,
		Few,
		Many
	};


	struct SuiteEntry
	{
		bool		bActive = true;
		bool		bWorking;
		string		name;
		int			nbTests;
		std::function<TestSuite*()> factory;
		TestSuite *	pReferenceSuite;
		TestSuite *	pTesteeSuite;
	};


	struct DeviceTest
	{
		Test* pTest = nullptr;
		double	render_time = 0;						// Seconds to render number of rounds
		double	stalling_time = 0;						// Seconds for endRender() call afterwards. OpenGL stalls here. 
	};

	struct CompareResult
	{
		bool	bDone = false;
		int		maxDiff = 0;							// Largest difference in any channel, 0-255.
		double	meanDiff = 0;							// Mean difference over all channels of all pixels.
		int		nbOverThreshold = 0;					// Channel values differing by more than c_diffThreshold.
		CoordI	worst;									// First pixel with the largest difference.
	};

	struct TestEntry
	{
		string	name;
		bool	bActive = false;
		bool	bWorking;

		DeviceTest devices[2];							// Testee, followed by Reference
		CompareResult compare;
	};

	// A test differs if any channel value differs by more than c_diffThreshold. If no more
	// than c_maxSpecks values do, the difference is reported as specks: isolated pixels
	// where backends may legitimately round differently, like samples right on a pixel edge.

	static const int	c_diffThreshold = 16;
	static const int	c_maxSpecks = 64;


	bool		setup_chrome();
	void		teardown_chrome();

	void		setup_testdevices();
	void		setup_tests();
	bool		add_testsuite( const std::function<TestSuite*()>& testSuiteFactory);
	void		regen_testentries();

	
	bool 		set_devices( Device_p pReference, Device_p pTestee );
	void		run_tests(Device* pDevice, DeviceEnum device);
	void		run_test(GfxDevice* pGfxDevice, Test* pTest);
	void		clock_test(DeviceTest* pDeviceTest, int rounds, Device* pDevice);
	void		destroy_tests();

	void		update_displaymode();
	void		display_test_results();

	void		setup_cliplist(ClipList list);

	SurfaceDisplay_p	create_canvas();
	void		refresh_performance_display();
	void		refresh_performance_measurements();

	void		run_comparison();
	bool		read_canvas(Device* pDevice, PixelFormat format, vector<uint8_t>& pixels);
	void		refresh_compare_display();
	void		show_test(int index);


	const SizeI			g_canvasSize = { 512, 512 };

	wapp::API*			m_pAPI = nullptr;
	wapp::Window_p		m_pWindow;

	ScrollCapsule_p		g_pViewScroller = nullptr;

	Device_p            g_pTesteeDevice = nullptr;
	Device_p            g_pReferenceDevice = nullptr;

	Widget_p			g_pPerformanceDisplay = nullptr;
	TablePanel_p		g_pPerformanceTable = nullptr;
	TextLayout_p		g_pPerformanceValueMapper = nullptr;
	PackLayout_p		g_pPerformanceEntryLayout = nullptr;

	vector<Device_p>	g_testdevices;
	vector<SuiteEntry>	g_testsuites;
	vector<TestEntry>	g_tests;

	vector<RectSPX>		g_clipList;

	DisplayMode			g_displayMode = DisplayMode::Testee;
	float				g_zoomFactor = 1.f;

	bool				g_bRefreshPerformance = false;

	Widget_p			g_pCompareDisplay = nullptr;
	TablePanel_p		g_pCompareTable = nullptr;
	TextDisplay_p		g_pCompareSummary = nullptr;
	TextStyle_p			g_pOkStyle = nullptr;
	TextStyle_p			g_pSpecksStyle = nullptr;
	TextStyle_p			g_pDiffStyle = nullptr;
	bool				g_bRunComparison = false;
	vector<RouteId>		g_compareRoutes;

	SelectCapsule_p		g_pTestSelector = nullptr;
	vector<Widget_p>	g_testListEntries;


	Surface_p			m_pLinearDeviceSurface;
	Surface_p			m_pLinearBackendSurface;

	Blob_p				m_pSavedBlob;
};
