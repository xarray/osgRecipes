SET(EXTERNAL_INCLUDE_DIR ./agg)
SET(EXAMPLE_NAME osgagg)
SET(EXAMPLE_FILES
    osgagg.cpp
    Drawer2D.cpp
    
    agg/agg_arc.cpp
    agg/agg_arrowhead.cpp
    agg/agg_bezier_arc.cpp
    agg/agg_bspline.cpp
    agg/agg_curves.cpp
    agg/agg_embedded_raster_fonts.cpp
    agg/agg_gsv_text.cpp
    agg/agg_image_filters.cpp
    agg/agg_line_aa_basics.cpp
    agg/agg_line_profile_aa.cpp
    agg/agg_rounded_rect.cpp
    agg/agg_sqrt_tables.cpp
    agg/agg_trans_affine.cpp
    agg/agg_trans_double_path.cpp
    agg/agg_trans_single_path.cpp
    agg/agg_trans_warp_magnifier.cpp
    agg/agg_vcgen_bspline.cpp
    agg/agg_vcgen_contour.cpp
    agg/agg_vcgen_dash.cpp
    agg/agg_vcgen_markers_term.cpp
    agg/agg_vcgen_smooth_poly1.cpp
    agg/agg_vcgen_stroke.cpp
    agg/agg_vpgen_clip_polygon.cpp
    agg/agg_vpgen_clip_polyline.cpp
    agg/agg_vpgen_segmentator.cpp
    
    agg/ctrl/agg_bezier_ctrl.cpp
    agg/ctrl/agg_cbox_ctrl.cpp
    agg/ctrl/agg_gamma_ctrl.cpp
    agg/ctrl/agg_gamma_spline.cpp
    agg/ctrl/agg_polygon_ctrl.cpp
    agg/ctrl/agg_rbox_ctrl.cpp
    agg/ctrl/agg_scale_ctrl.cpp
    agg/ctrl/agg_slider_ctrl.cpp
    agg/ctrl/agg_spline_ctrl.cpp
    agg/gpc/gpc.c
)

IF(WIN32)
    SET(EXAMPLE_FILES ${EXAMPLE_FILES}
        agg/font_win32_tt/agg_font_win32_tt.cpp
    )
ENDIF(WIN32)

IF(FREETYPE_FOUND)
    SET(EXTERNAL_INCLUDE_DIR ${EXTERNAL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
    SET(EXTERNAL_LIBRARIES ${FREETYPE_LIBRARIES})
    SET(EXAMPLE_FILES ${EXAMPLE_FILES}
        agg/font_freetype/agg_font_freetype.cpp
    )
    ADD_DEFINITIONS(-DOSGAGG_USE_FREETYPE)
ENDIF(FREETYPE_FOUND)
START_EXAMPLE()
