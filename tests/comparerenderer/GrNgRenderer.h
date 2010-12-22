/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#pragma once

#include <new>
#include "Renderer.h"
#include <graphite2/Types.h>
#include <graphite2/Segment.h>

namespace gr2 = org::sil::graphite::v2;

class GrNgRenderer : public Renderer
{
public:
    GrNgRenderer(const char * fontFile, int fontSize, int textDir, int cache)
        : m_rtl(textDir),
        m_grFace((cache == 0)?
            gr2::gr_make_file_face(fontFile, gr2::gr_face_preloadGlyphs):
            gr2::gr_make_file_face_with_seg_cache(fontFile, cache,
                gr2::gr_face_cacheCmap | gr2::gr_face_preloadGlyphs)),
        m_grFont(0)
    {
        if (m_grFace)
        {
            m_grFont = gr2::gr_make_font(static_cast<float>(fontSize), m_grFace);
        }
    }
    virtual ~GrNgRenderer()
    {
        gr2::gr_font_destroy(m_grFont);
        gr2::gr_face_destroy(m_grFace);
        m_grFont = NULL;
        m_grFace = NULL;
    }
    virtual void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        const void * pError = NULL;
        if (!m_grFace)
        {
            new(result) RenderedLine();
            return;
        }
        size_t numCodePoints = gr2::gr_count_unicode_characters(gr2::gr_utf8,
            reinterpret_cast<const void*>(utf8), reinterpret_cast<const void*>(utf8 + length), &pError);
        if (pError)
            fprintf(stderr, "Invalid Unicode pos %d\n", static_cast<int>(reinterpret_cast<const char*>(pError) - utf8));
        gr2::GrSegment* pSeg = gr2::gr_make_seg(m_grFont, m_grFace, 0u, NULL, gr2::gr_utf8, utf8, numCodePoints, m_rtl);
        if (!pSeg)
        {
            fprintf(stderr, "Failed to create segment\n");
            new(result) RenderedLine(0, .0f);
            return;
        }
        RenderedLine * renderedLine = new(result) RenderedLine(gr2::gr_seg_n_slots(pSeg),
                                                               gr2::gr_seg_advance_X(pSeg));
        int i = 0;
        for (const gr2::GrSlot* s = gr2::gr_seg_first_slot(pSeg); s;
             s = gr2::gr_slot_next_in_segment(s), ++i)
            (*renderedLine)[i].set(gr2::gr_slot_gid(s), gr2::gr_slot_origin_X(s),
                                   gr2::gr_slot_origin_Y(s), gr2::gr_slot_before(s),
                                   gr2::gr_slot_after(s));
        gr2::gr_seg_destroy(pSeg);
    }
    virtual const char * name() const { return "graphiteng"; }
private:
    int m_rtl;
    gr2::GrFace * m_grFace;
    gr2::GrFont * m_grFont;
};
