/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_FLEXPANEL_DOT_H
#define WG_FLEXPANEL_DOT_H
#pragma once

#include <wg_panel.h>

namespace wg
{

	class FlexPanel;
	typedef	StrongPtr<FlexPanel>	FlexPanel_p;
	typedef	WeakPtr<FlexPanel>		FlexPanel_wp;


	//____ FlexPos ____________________________________________________________


	class FlexPos
	{
	public:

		//.____ Creation _______________________________________________________

		inline FlexPos() {}

		inline FlexPos( Placement _origo ) : origo(s_origoTab[(int)_origo][0], s_origoTab[(int)_origo][1]) {}
		inline FlexPos( Placement _origo, Coord _offset ) : origo(s_origoTab[(int)_origo][0], s_origoTab[(int)_origo][1]), offset(_offset) {}

		inline FlexPos( CoordF _origo ) : origo(_origo) {}
		inline FlexPos( CoordF _origo, Coord _offset ) : origo(_origo), offset(_offset) {}

		inline FlexPos( float _origoX, float _origoY ) : origo(_origoX, _origoY) {}
		inline FlexPos( float _origoX, float _origoY, Coord _offset ) : origo(_origoX, _origoY), offset(_offset) {}

		//.____ Geometry _______________________________________________________

		Coord	pos(Size canvas) const
		{
			Coord c( origo.x*canvas.w + offset.x, origo.y*canvas.h + offset.y);
			return c;
		}

		//.____ Properties _____________________________________________________

		CoordF	origo;
		Coord	offset;

		//.____ Operators ______________________________________________________

		bool			operator==(const FlexPos& other) const { return origo == other.origo &&
																			offset == other.offset; }
		bool			operator!=(const FlexPos& other) const { return origo != other.origo ||
																			offset != other.offset; }
		//.____ Internal _______________________________________________________

		CoordSPX	_pos(SizeSPX canvas,  int scale) const
		{
			CoordSPX c(origo.x * canvas.w + offset.x*scale, origo.y * canvas.h + offset.y*scale);
			return c;
		}

	private:
		static float	s_origoTab[Placement_size][2];
	};


	//____ FlexPanelSlot ____________________________________________________________

	class FlexPanelSlot : public PanelSlot
	{
		friend class FlexPanel;
		template<class S> friend class DynamicSlotVector;

	public:

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			// We slightly deviate from keeping things in alphabetical order here
			// to keep the two alternatives grouped. Doesn't matter though,
			// all members that should be used in same blueprint still comes in alphabetical order.

			// Set for pinned geo only.

			FlexPos		pin1 = Placement::Undefined;
			FlexPos		pin2 = Placement::Undefined;

			// Set for movable geo only.

			FlexPos		hotspot = Placement::Undefined;
			FlexPos		origo = Placement::NorthWest;
			Coord		pos;
			Size		size;

			// Set for both

			bool		visible = true;
		};


		//.____ Identification ________________________________________________

		const static TypeInfo	TYPEINFO;

		//.____ Properties _________________________________________________________

		void			setPinned();
		void			setPinned(const FlexPos& topLeft, const FlexPos& bottomRight);
		inline bool		isPinned() const { return m_bPinned; }

		void			setMovable(const FlexPos& origo = Placement::NorthWest, const FlexPos& hotspot = Placement::Undefined);
		void			setMovable(const Rect& geometry, const FlexPos& origo, const FlexPos& hotspot = Placement::Undefined);
		inline bool		isMovable() const { return !m_bPinned; }


		//.____ Geometry ______________________________________________________

		// Methods for movable children

		bool			setOrigo(const FlexPos& origo);
		inline FlexPos	origo() const { return m_origo; }

		bool			setHotspot(const FlexPos& hotspot);
		inline FlexPos	hotspot() const { return m_hotspot; }

		bool			setGeo(const Rect& geometry);

		bool			setOffset(const Coord& ofs);
		inline Coord	offset() const { return m_placementGeo.pos(); }

		bool			setSize(const Size& size);

		bool			move(const Coord& ofs);

		// Methods for pinned children

		inline FlexPos	topLeftPin() const { return m_topLeftPin; }
		inline FlexPos	bottomRightPin() const { return m_bottomRightPin; }


		//.____ Operators __________________________________________

		inline void operator=(Widget * pWidget) { setWidget(pWidget); }

		//.____ Misc _______________________________________________

		using Link = DynamicSlotVector<FlexPanelSlot>::SlotLink;

	protected:

		FlexPanelSlot(SlotHolder * pHolder) : PanelSlot(pHolder) {}

		FlexPanelSlot(FlexPanelSlot&& o) = default;
		FlexPanelSlot& operator=(FlexPanelSlot&& o) = default;


		bool _setBlueprint( const Blueprint& bp );

//		inline FlexPanel * _holder() { return static_cast<FlexPanel*>(DynamicSlot::_holder()); }
//		inline const FlexPanel * _holder() const { return static_cast<const FlexPanel*>(DynamicSlot::_holder()); }

		bool			m_bPinned = false;

		// Positioned children

		FlexPos			m_origo = Placement::NorthWest;
		FlexPos			m_hotspot = Placement::NorthWest;
		Rect			m_placementGeo;		// Widgets geo relative origo and hotspot, not pixel aligned.

		//	Stretched children

		FlexPos			m_topLeftPin;
		FlexPos			m_bottomRightPin;

	};


	//____ FlexPanel _________________________________________________________

	/**
	 * @brief	A widget for individual, flexible arrangement of children.
	 *
	 * A widget for individual, flexible arrangement of children.
	 *
	 */


	class FlexPanel : public Panel<FlexPanelSlot>
	{
		friend class FlexPanelSlot;

	public:

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			Size			defaultSize;
			bool			disabled		= false;
			bool			dropTarget		= false;
			EdgePolicy		edgePolicy		= EdgePolicy::Confine;
			Finalizer_p		finalizer		= nullptr;
			int				id				= 0;
			MarkPolicy		markPolicy		= MarkPolicy::AlphaTest;
			MaskOp			maskOp			= MaskOp::Recurse;
			bool			pickable		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			PointerStyle	pointer			= PointerStyle::Undefined;
			bool			selectable		= true;
			Skin_p			skin;
			bool			stickyFocus		= false;
			bool			tabLock			= false;
			String			tooltip;
			bool			usePickHandles	= false;

		};

		//.____ Creation __________________________________________

		static FlexPanel_p	create() { return FlexPanel_p(new FlexPanel()); }
		static FlexPanel_p	create( const Blueprint& blueprint ) { return FlexPanel_p(new FlexPanel(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&	typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ________________________________________________________

		void			setDefaultSize( Size size );
		bool			isDefaultSizeSet() const { return !m_defaultSize.isEmpty(); }

		//.____ Layout __________________________________________________________

		void			setEdgePolicy(EdgePolicy policy);
		EdgePolicy		edgePolicy() const { return m_edgePolicy; }

		//.____ Internal ______________________________________________________

		SizeSPX			_defaultSize(int scale) const override;

	protected:
		FlexPanel();
		template<class BP> FlexPanel(const BP& bp) : Panel(bp)
		{
			m_bSiblingsOverlap = true;
			m_edgePolicy = bp.edgePolicy;
            m_defaultSize = bp.defaultSize;
        }

        virtual ~FlexPanel();

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		void		_childRequestRender( StaticSlot * pSlot, const RectSPX& rect ) override;
		void		_childRequestResize( StaticSlot * pSlot ) override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewChild ) override;

		void		_didAddSlots(StaticSlot * pSlot, int nb) override;
		void		_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
		void		_willEraseSlots(StaticSlot * pSlot, int nb) override;
		void		_hideSlots(StaticSlot *, int nb) override;
		void		_unhideSlots(StaticSlot *, int nb) override;

		
	private:
		void		_refreshRealGeo(FlexPanelSlot * pSlot, bool bForceResize = false, bool bUpdateInfluence = true);
		SizeSPX		_size() const { return m_size; }

		void		_resize( const SizeSPX& size, int scale ) override;

		void		_onRequestRender( const RectSPX& rect, const FlexPanelSlot * pSlot );

		SizeSPX		_sizeNeededForGeo( FlexPanelSlot * pSlot, int scale ) const;

		EdgePolicy	m_edgePolicy = EdgePolicy::Confine;		// In confine widget mode we don't clip overflow.
		Size		m_defaultSize;
	};



} // namespace wg
#endif //WG_FLEXPANEL_DOT_H
