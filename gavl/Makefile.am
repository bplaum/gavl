
if HAVE_MMX
mmx_libs = mmx/libgavl_mmx.la mmxext/libgavl_mmxext.la
mmx_subdirs = mmx mmxext
else
mmx_libs = 
mmx_subdirs =
endif

if HAVE_SSE
sse_libs = sse/libgavl_sse.la
sse_subdirs = sse
else
sse_libs = 
sse_subdirs =
endif

if HAVE_SSE2
sse2_libs = sse2/libgavl_sse2.la
sse2_subdirs = sse2
else
sse2_libs = 
sse2_subdirs =
endif

if HAVE_SSE3
sse3_libs = sse3/libgavl_sse3.la
sse3_subdirs = sse3
else
sse3_libs = 
sse3_subdirs =
endif


if HAVE_LIBVA
hw_libva_sources = hw_vaapi.c
hw_libva_cflags = @LIBVA_CFLAGS@
hw_libva_libs   = @LIBVA_LIBS@
else
hw_libva_sources = 
hw_libva_cflags =
hw_libva_libs   =
endif

if HAVE_GL
hw_gl_sources = hw_gl.c
hw_gl_cflags = @GL_CFLAGS@
hw_gl_libs =   @GL_LIBS@
else
hw_gl_sources = 
hw_gl_cflags = 
hw_gl_libs = 
endif

if HAVE_EGL
hw_egl_sources = hw_egl.c
hw_egl_cflags = @EGL_CFLAGS@
hw_egl_libs =   @EGL_LIBS@
else
hw_egl_sources = 
hw_egl_cflags = 
hw_egl_libs = 
endif

if HAVE_V4L2
hw_v4l2_sources = hw_v4l2.c
else
hw_v4l2_sources = 
endif

AM_CFLAGS = \
@LIBGAVL_CFLAGS@ \
@DRM_CFLAGS@ \
$(hw_libva_cflags) \
$(hw_gl_cflags) \
$(hw_egl_cflags) \
-I/usr/include

SUBDIRS = \
hq \
c \
libgdither \
libsamplerate \
$(mmx_subdirs) \
$(sse_subdirs) \
$(sse2_subdirs) \
$(sse3_subdirs) \
$(threednow_subdirs)

lib_LTLIBRARIES= libgavl.la
#noinst_LTLIBRARIES = libgavl.la

libgavl_la_SOURCES = \
$(hw_libva_sources) \
$(hw_egl_sources) \
$(hw_gl_sources) \
$(hw_v4l2_sources) \
absdiff.c \
arith128.c \
array.c \
audioconverter.c \
audioformat.c \
audioframe.c \
audiooptions.c \
audiosink.c \
audiosource.c \
base64.c \
bayer.c \
blend.c \
buffer.c \
chapterlist.c \
charset.c \
colorchannel.c \
colorspace.c \
compression.c \
countrycodes.c \
cputest.c \
deinterlace.c \
deinterlace_blend.c \
deinterlace_copy.c \
deinterlace_scale.c \
dictionary.c \
dsp.c \
dsputils.c \
edl.c \
frametable.c \
hw.c \
hw_dmabuf.c \
hw_shm.c \
http.c \
httpclient.c \
interleave.c \
io_buffer.c \
io.c \
io_cipher.c \
io_fd.c \
io_mem.c \
io_socket.c \
io_stdio.c \
io_tls.c \
language.c \
log.c \
md5.c \
memalign.c \
memcpy.c \
metadata.c \
mix.c \
msg.c \
numptr.c \
orientation.c \
packet.c \
packetbuffer.c \
packetindex.c \
packetsink.c \
packetsource.c \
palette.c \
parameter.c \
peakdetector.c \
psnr.c \
ptscache.c \
rectangle.c \
sampleformat.c \
samplerate.c \
scale.c \
scale_context.c \
scale_kernels.c \
scale_table.c \
shm.c \
socket.c \
socketaddress.c \
ssim.c \
state.c \
stats.c \
time.c \
timecode.c \
timer.c \
threadpool.c \
trackinfo.c \
transform.c \
transform_context.c \
transform_table.c \
urlvars.c \
utils.c \
videoconverter.c \
videoformat.c \
videoframe.c \
videosink.c \
videosource.c \
videooptions.c \
value.c \
volume.c

noinst_HEADERS = \
csp_packed_packed.h  \
csp_packed_planar.h  \
csp_planar_packed.h  \
csp_planar_planar.h \
ssim_tab.h

AM_LDFLAGS = -export-symbols-regex 'gav(l|f)' -version-info \
@LTVERSION_CURRENT@:@LTVERSION_REVISION@:@LTVERSION_AGE@ \
@LIBGAVL_LDFLAGS@ @GMERLIN_LIB_LDFLAGS@

libgavl_la_LIBADD = \
$(mmx_libs) \
$(sse_libs) \
$(sse2_libs) \
$(sse3_libs) \
c/libgavl_c.la \
hq/libgavl_hq.la \
libgdither/libgdither.la \
libsamplerate/libsamplerate.la \
@LIBGAVL_LIBS@ \
@DRM_LIBS@ \
$(hw_libva_libs) \
$(hw_gl_libs) \
$(hw_egl_libs) \
-lpthread \
-lm

EXTRA_DIST=Makefile.am.in
