#include <iostream>
#include <vector>

#include <wg_types.h>


class KernelDB
{
public:
	KernelDB();
	~KernelDB();

	enum class BlitType
	{
		StraightBlit,
		StraightTile,
		StraightBlur,

		TransformBlitNearest,
		TransformClipBlitNearest,
		TransformTileNearest,
		TransformBlurNearest,

		TransformBlitBilinear,
		TransformClipBlitBilinear,
		TransformTileBilinear,
		TransformBlurBilinear
	};

	const static int BlitType_size = 11;


	struct CustomBlitSpec
	{
		CustomBlitSpec();

		bool			blitTypes[BlitType_size];
		bool			tintModes[wg::TintMode_size];
		bool			blendModes[wg::BlendMode_size];
		bool			sourceFormats[wg::PixelFormat_size];
		bool			destFormats[wg::PixelFormat_size];
	};

	struct KernelCount
	{
		int fill = 0;
		int line = 0;
		int clipLine = 0;
		int segment = 0;

		int	pass1blits_straight = 0;
		int pass1blits_straight_fast8 = 0;

		int	pass1blits_transform = 0;
		int pass1blits_transform_fast8 = 0;

		int pass2blits = 0;
		int pass2blits_fast8 = 0;

		int	customBlits = 0;
	};


	void	reset();
	void	clear();

	bool	generateSource( std::ostream& filename, const std::string& kernelLabel );
	KernelCount	countKernels();
	int		countEntryKernels(int entry);

	void	setTintMode(wg::TintMode mode, bool bOn);
	void	setBlendMode(wg::BlendMode mode, bool bOn);
	void	setSrcFormat(wg::PixelFormat format, bool bOn);
	void	setDestFormat(wg::PixelFormat format, bool bOn);

	bool	tintMode(wg::TintMode mode) { return m_tintModes[int(mode)]; }
	bool	blendMode(wg::BlendMode mode) { return m_blendModes[int(mode)]; }
	bool	srcFormat(wg::PixelFormat format) { return m_srcFormats[int(format)]; }
	bool	destFormat(wg::PixelFormat format) { return m_destFormats[int(format)]; }

	const bool*	tintModes() { return m_tintModes; }
	const bool* blendModes() { return m_blendModes; }
	const bool* srcFormats() { return m_srcFormats; }
	const bool* destFormats() { return m_destFormats; }

	std::vector<CustomBlitSpec>::const_iterator beginCustomBlits() const { return m_customBlits.begin(); }
	std::vector<CustomBlitSpec>::const_iterator endCustomBlits() const { return m_customBlits.end(); }

	CustomBlitSpec* customBlitEntry(int index) { return &m_customBlits.at(index); };

	void	addCustomBlitEntry() { m_customBlits.push_back( CustomBlitSpec() ); }
	void	deleteCustomBlitEntry(int index) { m_customBlits.erase( m_customBlits.begin()+index ); }



private:

	bool	m_tintModes[wg::TintMode_size];
	bool	m_blendModes[wg::BlendMode_size];
	bool	m_srcFormats[wg::PixelFormat_size];
	bool	m_destFormats[wg::PixelFormat_size];

	std::vector<CustomBlitSpec>	m_customBlits;



/*
	// TintMode, BlendMode, SourceFormat, DestFormat, bTile

	bool	m_simpleBlitMap[wg::TintMode_size][wg::BlendMode_size][wg::PixelFormat_size][wg::PixelFormat_size][2];

	// TintMode, BlendMode, SourceFormat, DestFormat, SampleMethod, Normal/Clip/Tile

	bool	m_complexBlitMap[wg::TintMode_size][wg::BlendMode_size][wg::PixelFormat_size][wg::PixelFormat_size][2][3];
*/


};
