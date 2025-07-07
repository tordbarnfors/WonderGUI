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

#ifndef WONDERGUI_DOT_H
#define WONDERGUI_DOT_H
#pragma once

#include <wondergfx.h>

#include <wg_animframes.h>
#include <wg_animplayer.h>
#include <wg_animskin.h>
#include <wg_areachart.h>
#include <wg_areascrollchart.h>
#include <wg_bakeskin.h>
#include <wg_base.h>
#include <wg_basicnumberlayout.h>
#include <wg_basictextlayout.h>
#include <wg_blockingcapsule.h>
#include <wg_blockskin.h>
#include <wg_blockslideskin.h>
#include <wg_boxskin.h>
#include <wg_bracketskin.h>
#include <wg_button.h>
#include <wg_canvascapsule.h>
#include <wg_canvasdisplay.h>
#include <wg_canvaslayers.h>
#include <wg_capsule.h>
#include <wg_caret.h>
#include <wg_char.h>
#include <wg_charbuffer.h>
#include <wg_charseq.h>
#include <wg_chart.h>
#include <wg_circleskin.h>
#include <wg_codepages.h>
#include <wg_colorskin.h>
#include <wg_columnheader.h>
#include <wg_component.h>
#include <wg_container.h>
#include <wg_lambdaskin.h>
#include <wg_dataset.h>
#include <wg_doubleskin.h>
#include <wg_dragndropoverlay.h>
#include <wg_dynamicslotvector.h>
#include <wg_dynamicvector.h>
#include <wg_editablenumber.h>
#include <wg_editabletext.h>
#include <wg_enumextras.h>
#include <wg_filler.h>
#include <wg_fillmeterskin.h>
#include <wg_flexpanel.h>
#include <wg_fpsdisplay.h>
#include <wg_framemeterskin.h>
#include <wg_glow.h>
#include <wg_hostbridge.h>
#include <wg_icon.h>
#include <wg_image.h>
#include <wg_inputhandler.h>
#include <wg_key.h>
#include <wg_knob.h>
#include <wg_labelcapsule.h>
#include <wg_lambdapanel.h>
#include <wg_lineeditor.h>
#include <wg_list.h>
#include <wg_modaloverlay.h>
#include <wg_msg.h>
#include <wg_msglogger.h>
#include <wg_msgrouter.h>
#include <wg_multiblockskin.h>
#include <wg_number.h>
#include <wg_numberdisplay.h>
#include <wg_numberlayout.h>
#include <wg_overlay.h>
#include <wg_packlayout.h>
#include <wg_packlist.h>
#include <wg_packpanel.h>
#include <wg_panel.h>
#include <wg_pianokeyboard.h>
#include <wg_piemeterskin.h>
#include <wg_plotchart.h>
#include <wg_popupopener.h>
#include <wg_popupoverlay.h>
#include <wg_rangeslider.h>
#include <wg_receiver.h>
#include <wg_renderlayercapsule.h>
#include <wg_reordercapsule.h>
#include <wg_root.h>
#include <wg_rootpanel.h>
#include <wg_scrollbar.h>
#include <wg_scroller.h>
#include <wg_scrollpanel.h>
#include <wg_selectcapsule.h>
#include <wg_blockslideskin.h>
#include <wg_scalecapsule.h>
#include <wg_selectbox.h>
#include <wg_sidecanvas.h>
#include <wg_sizecapsule.h>
#include <wg_skin.h>
#include <wg_skinslot.h>
#include <wg_skinslotmanager.h>
#include <wg_skinslotmi.h>
#include <wg_slider.h>
#include <wg_slot.h>
#include <wg_slotarray.h>
#include <wg_slotholder.h>
#include <wg_spinanimskin.h>
#include <wg_spinmeterskin.h>
#include <wg_splitpanel.h>
#include <wg_stackpanel.h>
#include <wg_state.h>
#include <wg_stateskin.h>
#include <wg_staticnumber.h>
#include <wg_staticslotvector.h>
#include <wg_statictext.h>
#include <wg_staticvector.h>
#include <wg_string.h>
#include <wg_surfacedisplay.h>
#include <wg_tablepanel.h>
#include <wg_text.h>
#include <wg_textbase.h>
#include <wg_textdisplay.h>
#include <wg_texteditor.h>
#include <wg_textlayout.h>
#include <wg_textlink.h>
#include <wg_textstyle.h>
#include <wg_textstylemanager.h>
#include <wg_texttool.h>
#include <wg_theme.h>
#include <wg_tileskin.h>
#include <wg_timer.h>
#include <wg_togglebutton.h>
#include <wg_togglegroup.h>
#include <wg_tooltipoverlay.h>
#include <wg_twoslotpanel.h>
#include <wg_types.h>
#include <wg_util.h>
#include <wg_widget.h>

#endif //WONDERGUI_DOT_H
