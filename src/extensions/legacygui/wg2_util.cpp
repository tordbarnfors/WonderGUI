
#include <wg2_util.h>
#include <wg2_geo.h>
#include <wg2_blockset.h>
#include <wg_surface.h>
#include <wg2_widget.h>
#include <wg2_panel.h>
#include <wg2_gfxanim.h>
#include <wg2_pen.h>
#include <assert.h>


bool WgUtil::AdjustScaledArea(const WgBlock& block, WgRect& area)
{
	if( block.IsFixedSize() )
	{
		int areaW = area.w;
		int areaH = area.h;

		int blockW = block.Width();
		int blockH = block.Height();

		if( areaW > blockW )
		{
			area.x += (areaW - blockW)/2;
			area.w = blockW;
		}

		if( areaH > blockH )
		{
			area.y += (areaH - blockH)/2;
			area.h = blockH;
		}

		return true;
	}


	if(!block.IsScaled())
		return false;

	const WgBorders& borders = block.CanvasFrame();

	int areaW = area.w - borders.width();
	int areaH = area.h - borders.height();

	int blockW = block.Width() - borders.width();
	int blockH = block.Height() - borders.height();

	if(areaW <= 0 || areaH <= 0 || blockW <= 0 || blockH <= 0)
		return false;

	int adjustedW = borders.width();
	int adjustedH = borders.height();

	if(areaW * blockH > blockW * areaH)
	{
		adjustedH += areaH;
		adjustedW += int(float(adjustedH * blockW) / float(blockH));
	}
	else
	{
		adjustedW += areaW;
		adjustedH += int(float(adjustedW * blockH) / float(blockW));
	}

	area.x += (area.w - adjustedW) >> 1;
	area.y += (area.h - adjustedH) >> 1;
	area.w = adjustedW;
	area.h = adjustedH;

	return true;
}


//____ MarkTestBlock() ________________________________________________________

bool WgUtil::MarkTestBlock( WgCoord ofs, const WgBlock& block, WgRect canvas, int opacityTreshold )
{
	if (!block.IsValid())
		return false;

	AdjustScaledArea(block, canvas);

	// Sanity check & shortcuts.
	if( !canvas.contains(ofs) )
		return false;

	int alpha;
	if( block.IsOpaque() )
		return ( 255 >= opacityTreshold);

	// Make cordinates relative canvas.

	ofs.x -= canvas.x;
	ofs.y -= canvas.y;


	const WgBorders& canvasFrame = block.CanvasFrame();
	const WgBorders& sourceFrame = block.SourceFrame();

	int outputScale = block.OutputScale();
	int surfaceScale = block.SurfaceScale();

	WgSize dimensions = block.Size();

	// Determine in which section the cordinate is (0-2 for x and y).

	int	xSection = 0;
	int ySection = 0;

	if( ofs.x >= canvas.w - canvasFrame.right )
		xSection = 2;
	else if( ofs.x > canvasFrame.left )
		xSection = 1;

	if( ofs.y >= canvas.h - canvasFrame.bottom )
		ySection = 2;
	else if( ofs.y > canvasFrame.top )
		ySection = 1;


	// Convert ofs.x to X-offset in bitmap, taking stretch/tile section into account.

	if( xSection == 0 )
	{
		ofs.x = (ofs.x * surfaceScale) / outputScale;
	}
	else if( xSection == 2 )
	{
		ofs.x = ofs.x - (canvas.w - canvasFrame.right);           // Offset in right border of canvas
		ofs.x = (ofs.x * surfaceScale) / outputScale;            // Scale from canvas to source coordinates
		ofs.x += dimensions.w - sourceFrame.right;          // Add offset for right border

//			ofs.x = dimensions.w - (canvas.w - ofs.x);
	}
	else if( xSection == 1 )
	{
		int tileAreaWidth = dimensions.w - sourceFrame.width();

		int canvasStretchWidth = canvas.w - canvasFrame.width();	// Width of stretch-area on screen.

		ofs.x = ofs.x - canvasFrame.left;               // Offset in middle section of canvas
//        ofs.x = (ofs.x * surfaceScale) / outputScale;        // Scale from canvas to source offset
		ofs.x = (int)((ofs.x / (float)canvasStretchWidth)*tileAreaWidth) + sourceFrame.left;
	}

	// Convert ofs.y to Y-offset in bitmap, taking stretch/tile section into account.

	if( ySection == 0 )
	{
		ofs.y = (ofs.y * surfaceScale) / outputScale;
	}
	if( ySection == 2 )
	{
		ofs.y = ofs.y - (canvas.h - canvasFrame.bottom);           // Offset in bottom border of canvas
		ofs.y = (ofs.y * surfaceScale) / outputScale;            // Scale from canvas to source coordinates
		ofs.y += dimensions.h - sourceFrame.bottom;          // Add offset for bottom border
	}
	else if( ySection == 1 )
	{
		int tileAreaHeight = dimensions.h - sourceFrame.height();
		int canvasStretchHeight = canvas.h - canvasFrame.height();	// Height of stretch-area on screen.

		ofs.y = ofs.y - canvasFrame.top;               // Offset in middle section of canvas
  //      ofs.y = (ofs.y * surfaceScale) / outputScale;        // Scale from canvas to source offset
		ofs.y = (int)((ofs.y / (float)canvasStretchHeight)*tileAreaHeight) + sourceFrame.top;
	}

	WgCoord srcOfs = block.Rect().pos();

	alpha = block.Surface()->alpha( WgCoord(srcOfs.x+ofs.x, srcOfs.y+ofs.y)*64 );

	return ( alpha*255/4096 >= opacityTreshold);
}



//____ SizeFromPolicy() __________________________________________________________

int WgUtil::SizeFromPolicy( int defaultSize, int specifiedSize, WgSizePolicy policy )
{
	switch( policy )
	{
		case wg::SizeConstraint::None:
			return defaultSize;
		case wg::SizeConstraint::Equal:
			return specifiedSize;
		case wg::SizeConstraint::LessOrEqual:
			if( defaultSize > specifiedSize )
				return specifiedSize;
		case wg::SizeConstraint::GreaterOrEqual:
			if( defaultSize < specifiedSize )
				return specifiedSize;
	}
	return defaultSize;
}

//____ OrigoToOfs() ________________________________________________________

WgCoord WgUtil::OrigoToOfs( WgOrigo origo, WgSize base )
{
	switch( origo )
	{
		default:
		case WgOrigo::NorthWest:
			return WgCoord(0,0);

		case WgOrigo::North:
			return WgCoord( base.w/2,0 );

		case WgOrigo::NorthEast:
			return WgCoord( base.w,0 );

		case WgOrigo::East:
			return WgCoord( base.w, base.h/2 );

		case WgOrigo::SouthEast:
			return WgCoord( base.w, base.h );

		case WgOrigo::South:
			return WgCoord( base.w/2, base.h );

		case WgOrigo::SouthWest:
			return WgCoord( 0, base.h );

		case WgOrigo::West:
			return WgCoord( 0, base.h/2 );

		case WgOrigo::Center:
			return WgCoord( base.w/2, base.h/2 );
	}
}

//____ OrigoToRect() ________________________________________________________

WgRect WgUtil::OrigoToRect( WgOrigo origo, WgSize base, WgSize rect )
{
	switch( origo )
	{
		default:
		case WgOrigo::NorthWest:
			return WgRect(0,0, rect);

		case WgOrigo::North:
			return WgRect( base.w/2 - rect.w/2, 0, rect );

		case WgOrigo::NorthEast:
			return WgRect( base.w - rect.w, 0, rect );

		case WgOrigo::East:
			return WgRect( base.w - rect.w, base.h/2 - rect.h/2, rect );

		case WgOrigo::SouthEast:
			return WgRect( base.w - rect.w, base.h - rect.h, rect );

		case WgOrigo::South:
			return WgRect( base.w/2 - rect.w/2, base.h - rect.h, rect );

		case WgOrigo::SouthWest:
			return WgRect( 0, base.h - rect.h, rect );

		case WgOrigo::West:
			return WgRect( 0, base.h/2 - rect.h/2, rect );

		case WgOrigo::Center:
			return WgRect( base.w/2 - rect.w/2, base.h/2 - rect.h/2, rect );
	}
}

//____ StateToMode() __________________________________________________________

WgMode WgUtil::StateToMode(WgState state)
{
	WgMode mode = WG_MODE_NORMAL;

	if (state.isDisabled())
		mode = WG_MODE_DISABLED;
	else if (state.isHovered() && state.isSelected())
		mode = WG_MODE_SPECIAL;
	else if (state.isPressed() || state.isSelected())
		mode = WG_MODE_SELECTED;
	else if (state.isHovered())
		mode = WG_MODE_MARKED;

	return mode;
}

//____ ModeToState() __________________________________________________________

WgState WgUtil::ModeToState(WgMode mode)
{
	WgState state = WgStateEnum::Default;

	switch( mode )
	{
		case WG_MODE_NORMAL:
			state = WgState::Default;
			break;
		case WG_MODE_MARKED:
			state = WgState::Hovered;
			break;
		case WG_MODE_SELECTED:
			state = WgState::SelectedPressed;
			break;
		case WG_MODE_SPECIAL:
			state = WgState::SelectedHovered;
			break;
		case WG_MODE_DISABLED:
			state = WgState::Disabled;
			break;

        case WG_MODE_ALL:
            assert(false);
	}

	return state;
}


//____ ParametricBlendInOut() __________________________________________________________

float WgUtil::ParametricBlendInOut(float t)
{
	float sqt = t*t;
	return sqt / (2.0f * (sqt - t) + 1.0f);
}

//____ lineWidth() ____________________________________________________________________

uint32_t WgUtil::lineWidth( void * pNode, const wg::TextAttr& attr, wg::State state, const wg::Char * pString, int scale )
{
	wg::TextAttr    attr2;

	WgPen pen;
    pen.SetScale(scale);
	Uint16 hStyle = 0xFFFF;

	while( !pString->isEndOfLine() )
	{
		if( pString->styleHandle() != hStyle )
		{
			attr2 = attr;

			auto pStyle = pString->stylePtr();

			if( pStyle )
				pStyle->addToAttr( state, &attr2, scale );
			pen.SetAttributes( attr2 );
		}
		pen.SetChar( pString->code() );
		pen.ApplyKerning();
		pen.AdvancePos();
		pString++;
	}

	// We include the terminator in case it is set to be visible.

	pen.SetChar( pString->code() );
	pen.ApplyKerning();
	pen.AdvancePos();

	return pen.GetPosX();
}
