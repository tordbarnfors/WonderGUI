#ifndef WG_TUNER_DOT_H
#define WG_TUNER_DOT_H

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#include <vector>
#include <map>
//____ WgTuner ____________________________________________________________

class WgTuner : public WgWidget
{
public:
	WgTuner();
	virtual ~WgTuner();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgTuner(); };

    enum pitch
    {
        flat,
        sharp,
        onNote,
        off,
    };
    //____ Methods __________________________________________

    //void			SetLEDColors( WgColor bottomOn, WgColor middleOn, WgColor topOn,
	//						   WgColor bottomOff, WgColor middleOff, WgColor topOff );
	//void			SetNbLEDs( int bottomSection, int middleSection, int topSection );

    void            SetLeds(int sections, std::vector<int> ledPerSection, std::map<int, std::pair<WgColor, WgColor>> sectionColor);

	void			SetLEDSpacing( float spacing );
	void			SetPitchValue( float peak, float hold );
    void            SetNoteValue(float peak, float hold);
    void			SetDirection( WgDirection direction );
	void            SetSidePadding( float padding ) { m_fSidePadding = padding; }
	void            SetZeroInMiddle(bool mb) { m_bZeroInMiddle = mb; }

	void            EnableFades(bool b) { m_bUseFades = b; }
	WgSize			PreferredPixelSize() const;
	void            SetActive(bool b) { m_bActive = b; }

	void            SetTunerColors(WgColor indicator, WgColor correct) { m_TunerIndicatorCol = indicator; m_TunerCorrectCol = correct; }
	void            SetTunerCorrect(bool b) { if(b != m_bTunerCorrect) { _requestRender(); } m_bTunerCorrect = b; }

    void            SetUseBackgroundColor(bool bUse) { m_bUseBackgroundColor = bUse; }
    void            SetBackgroundColor(WgColor kColor) { m_BackgroundColor = kColor; }
    pitch           GetPitch() { return m_ePitch; }

    std::string     GetNote() { return m_kNote; };

protected:
    void			_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	void			_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	void			_onCloneContent( const WgWidget * _pOrg );
	bool			_onAlphaTest( const WgCoord& ofs );
	void			_onNewSize( const WgSize& size );


	WgDirection		m_direction;
//	WgColor			m_LEDColors[3][2];
//	int				m_nSectionLEDs[3];
    std::vector<int> m_nSectionLEDs;
    std::map<int, std::pair<WgColor, WgColor> > m_LEDColors;
	int				m_nLEDs;
	float			m_LEDSpacing;

	int				m_iPeak;
	int				m_iHold;

	int				m_iNotePeak;
	int				m_iNoteHold;

	float           m_fPeak;
	float           m_fNotePeak;

	float           m_fSidePadding;
	int             m_iSidePadding;

	bool            m_bZeroInMiddle;
	float           m_d;
	float           m_d2;

	bool            m_bTunerMode = true;
	WgColor         m_TunerIndicatorCol;
	WgColor         m_TunerCorrectCol;
	bool            m_bTunerCorrect = false;

	bool            m_bActive = true;

    int         m_iNoteMillisec = -1.f;
    int         m_iTickMillisec = 0;
    int         m_iDelay = -1;

    std::vector<float> m_LEDStates;

	bool            m_bUseFades;
    bool            m_bUseBackgroundColor;
    WgColor         m_BackgroundColor;

    std::vector<float> m_kFilter;
    int m_iFilterSize = 7;
    int m_iFilterPos = 0;

    pitch m_ePitch = pitch::off;
    const float m_fPitchtolerance = 0.01f;

    std::vector<float> m_kNoteFilter;
    int m_iNoteFilterSize = 31;
    int m_iNoteFilterPos = 0;

    std::string m_kNote = "";
	const std::string g_sTone[70] = { 	reinterpret_cast<const char *>(u8"G\u266f0"),"A0",
										reinterpret_cast<const char *>(u8"A\u266f0"),"B0","C1",
										reinterpret_cast<const char *>(u8"C\u266f1"), "D1",
										reinterpret_cast<const char *>(u8"E\u266d1"), "E1", "F1",
										reinterpret_cast<const char *>(u8"F\u266f1"), "G1",
										reinterpret_cast<const char *>(u8"G\u266f1"), "A1",
										reinterpret_cast<const char *>(u8"A\u266f1"), "B1", "C2",
										reinterpret_cast<const char *>(u8"C\u266f2"), "D2",
										reinterpret_cast<const char *>(u8"E\u266d2"), "E2", "F2",
										reinterpret_cast<const char *>(u8"F\u266f2"), "G2",
										reinterpret_cast<const char *>(u8"G\u266f2"), "A2",
										reinterpret_cast<const char *>(u8"A\u266f2"), "B2", "C3",
										reinterpret_cast<const char *>(u8"C\u266f3"), "D3",
										reinterpret_cast<const char *>(u8"E\u266d3"), "E3", "F3",
										reinterpret_cast<const char *>(u8"F\u266f3"), "G3",
										reinterpret_cast<const char *>(u8"G\u266f3"), "A3",
										reinterpret_cast<const char *>(u8"A\u266f3"), "B3", "C4",
										reinterpret_cast<const char *>(u8"C\u266f4"), "D4",
										reinterpret_cast<const char *>(u8"E\u266d4"), "E4", "F4",
										reinterpret_cast<const char *>(u8"F\u266f4"), "G4",
										reinterpret_cast<const char *>(u8"G\u266f4"), "A4",
										reinterpret_cast<const char *>(u8"A\u266f4"), "B4", "C5",
										reinterpret_cast<const char *>(u8"C\u266f5"), "D5",
										reinterpret_cast<const char *>(u8"E\u266d5"), "E5", "F5",
										reinterpret_cast<const char *>(u8"F\u266f5"), "G5",
										reinterpret_cast<const char *>(u8"G\u266f5"), "A5",
										reinterpret_cast<const char *>(u8"A\u266f5"), "B5", "C6",
										reinterpret_cast<const char *>(u8"C\u266f6"), "D6",
										reinterpret_cast<const char *>(u8"E\u266d6"), "E6", "F6" };
};

#endif //WG_TUNER_DOT_H
