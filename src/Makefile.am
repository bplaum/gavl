AM_LDFLAGS = @GMERLIN_EXE_LDFLAGS@

if HAVE_LIBVA_X11
libva_x11_programs = vaapi_x11
else
libva_x11_programs = 
endif


if HAVE_V4L2
v4l2_programs = v4linfo v4l_m2m_converter
else
v4l2_programs = 
endif



if HAVE_LIBPNG
png_programs = \
colorspace_test \
fill_test \
blend_test \
scaletest \
convolvetest \
deinterlacetest
else
png_programs =
endif

noinst_PROGRAMS = \
$(png_programs) \
$(libva_x11_programs) \
$(v4l2_programs) \
benchmark \
charset \
colorspace_time \
deinterlace_time \
dma2egl \
dump_frame_table \
httptest \
orientationtest \
pixelformat_penalty \
plot_scale_kernels \
scale_time \
timescale_test \
value_test \
volume_test

pixelformat_penalty_SOURCES = pixelformat_penalty.c
pixelformat_penalty_LDADD = ../gavl/libgavl.la

vaapi_x11_CFLAGS = @X_CFLAGS@ @LIBVA_CFLAGS@
vaapi_x11_SOURCES = vaapi_x11.c
vaapi_x11_LDADD = ../gavl/libgavl.la @LIBVA_LIBS@ @X_LIBS@

dma2egl_SOURCES = dma2egl.c
dma2egl_LDADD = ../gavl/libgavl.la @X_LIBS@

benchmark_SOURCES = benchmark.c
benchmark_LDADD = ../gavl/libgavl.la @RT_LIBS@

charset_SOURCES = charset.c
charset_LDADD = ../gavl/libgavl.la

volume_test_SOURCES = volume_test.c
volume_test_LDADD = -lm ../gavl/libgavl.la

value_test_SOURCES = value_test.c
value_test_LDADD = -lm ../gavl/libgavl.la

dump_frame_table_SOURCES = dump_frame_table.c
dump_frame_table_LDADD = ../gavl/libgavl.la

v4linfo_SOURCES = v4linfo.c
v4linfo_LDADD = -lm ../gavl/libgavl.la

v4l_m2m_converter_SOURCES = v4l_m2m_converter.c
v4l_m2m_converter_LDADD = -lm ../gavl/libgavl.la

colorspace_test_SOURCES = colorspace_test.c
colorspace_test_LDADD = ../gavl/libgavl.la -lpng -lz

fill_test_SOURCES = fill_test.c
fill_test_LDADD = ../gavl/libgavl.la -lpng -lz

blend_test_SOURCES = blend_test.c
blend_test_LDADD = ../gavl/libgavl.la -lpng -lz

scaletest_SOURCES = scaletest.c
scaletest_LDADD = ../gavl/libgavl.la -lpng -lz

convolvetest_SOURCES = convolvetest.c pngutil.c
convolvetest_LDADD = ../gavl/libgavl.la -lpng -lz

deinterlacetest_SOURCES = deinterlacetest.c pngutil.c
deinterlacetest_LDADD = ../gavl/libgavl.la -lpng -lz

orientationtest_SOURCES = orientationtest.c pngutil.c
orientationtest_LDADD = ../gavl/libgavl.la -lpng -lz

httptest_SOURCES = httptest.c
httptest_LDADD = ../gavl/libgavl.la -lpng -lz


plot_scale_kernels_SOURCES = plot_scale_kernels.c
plot_scale_kernels_LDADD = ../gavl/libgavl.la

scale_time_SOURCES = scale_time.c timeutils.c
scale_time_LDADD = ../gavl/libgavl.la

deinterlace_time_SOURCES = deinterlace_time.c timeutils.c
deinterlace_time_LDADD = ../gavl/libgavl.la

colorspace_time_SOURCES = colorspace_time.c
colorspace_time_LDADD = ../gavl/libgavl.la

timescale_test_SOURCES = timescale_test.c
timescale_test_LDADD = ../gavl/libgavl.la

noinst_HEADERS = pngutil.h timeutils.h

EXTRA_DIST = \
gen_colorspace_tables.c \
gen_ssim_table.c
