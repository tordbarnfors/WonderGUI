

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


	bool	init(Visitor* pVisitor) override;
	bool	update() override;
	void	exit() override;

	

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
		Time
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

	struct TestEntry
	{
		string	name;
		bool	bActive = false;
		bool	bWorking;

		DeviceTest devices[2];							// Testee, followed by Reference
	};


	bool		setup_theme();
	bool		setup_chrome();
	void		teardown_chrome();

	void		setup_testdevices();
	void		setup_tests();
	bool		add_testsuite( const std::function<TestSuite*()>& testSuiteFactory);
	void		regen_testentries();

	
	bool 		set_devices( Device_p pReference, Device_p pTestee );
	void		run_tests(Device* pDevice, DeviceEnum device);
	void		clock_test(DeviceTest* pDeviceTest, int rounds, Device* pDevice);
	void		destroy_tests();

	void		update_displaymode();
	void		display_test_results();

	void		setup_cliplist(ClipList list);

	SurfaceDisplay_p	create_canvas();
	void		refresh_performance_display();
	void		refresh_performance_measurements();


	const SizeI			g_canvasSize = { 512, 512 };

	Visitor*			m_pVisitor = nullptr;
	Window_p			m_pWindow;

	ScrollPanel_p		g_pViewPanel = nullptr;

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

	TextStyle_p			g_pButtonLabelStyle = nullptr;
	TextLayout_p		g_pButtonLabelMapper = nullptr;

	DisplayMode			g_displayMode = DisplayMode::Testee;
	float				g_zoomFactor = 1.f;

	bool				g_bRefreshPerformance = false;


	Surface_p			m_pLinearDeviceSurface;
	Surface_p			m_pLinearBackendSurface;

	Blob_p				m_pSavedBlob;
};
