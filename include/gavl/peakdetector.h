/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/

/**
 * @file peakdetector.h
 * external api header.
 */

#ifndef GAVL_PEAKDETECTORS_H_INCLUDED
#define GAVL_PEAKDETECTORS_H_INCLUDED

#include <gavl/connectors.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup peak_detection Peak detector
 *  \ingroup audio
 *  \brief Detect peaks in the volume for steering normalizers and
 *    dynamic range compressors
 *   While normalizers and dynamic range controls are out of the scope
 *   of gavl, some low-level functionality can be provided
 *
 * @{
 */
 
/*! \brief Opaque structure for peak detector
 *
 * You don't want to know what's inside.
 */

typedef struct gavl_peak_detector_s gavl_peak_detector_t;
  
/* Create / destroy */

/*! \brief Callback for getting the peaks across all channels
 *  \param priv Client data
 *  \param samples Number of samples of last update call
 *  \param min Minimum value (scaled betwen -1.0 and 1.0)
 *  \param min Maximum value (scaled betwen -1.0 and 1.0)
 *  \param abs Absolute value (scaled betwen 0.0 and 1.0)
 *
 *  Since 1.5.0
 */

typedef void (*gavl_update_peak_callback)(void * priv,
                                          int samples,
                                          double min, double max, double abs);

/*! \brief Callback for getting the peaks for all channels separately
 *  \param priv Client data
 *  \param samples Number of samples of last update call
 *  \param min Minimum value (scaled betwen -1.0 and 1.0)
 *  \param min Maximum value (scaled betwen -1.0 and 1.0)
 *  \param abs Absolute value (scaled betwen 0.0 and 1.0)
 *
 *  Since 1.5.0
 */
 
typedef void (*gavl_update_peaks_callback)(void * priv,
                                           int samples,
                                           const double * min,
                                           const double * max,
                                           const double * abs);

/*! \brief Create peak detector
 *  \returns A newly allocated peak detector
 */
  
GAVL_PUBLIC
gavl_peak_detector_t * gavl_peak_detector_create();

/*! \brief Set callbacks
 *  \param pd A peak detector
 *  \param peak_callback Callback for overall peaks or NULL
 *  \param peaks_callback Callback for per channel peaks or NULL
 *  \param priv Client data passed to the callbacks
 *
 *  Since 1.5.0
 */
  
GAVL_PUBLIC
void gavl_peak_detector_set_callbacks(gavl_peak_detector_t * pd,
                                      gavl_update_peak_callback peak_callback,
                                      gavl_update_peaks_callback peaks_callback,
                                      void * priv);

  
/*! \brief Destroys a peak detector and frees all associated memory
 *  \param pd A peak detector
 */

GAVL_PUBLIC
void gavl_peak_detector_destroy(gavl_peak_detector_t *pd);

/*! \brief Set format for a peak detector
 *  \param pd A peak detector
 *  \param format The format subsequent frames will be passed with
 *
 * This function can be called multiple times with one instance. It also
 * calls \ref gavl_peak_detector_reset.
 */

GAVL_PUBLIC
void gavl_peak_detector_set_format(gavl_peak_detector_t *pd,
                                   const gavl_audio_format_t * format);

/*! \brief Get format
 *  \param pd A peak detector
 *  \returns The internal format
 *
 *  Since 1.5.0
 */
  
GAVL_PUBLIC const gavl_audio_format_t *
gavl_peak_detector_get_format(gavl_peak_detector_t * pd);

  
/*! \brief Feed the peak detector with a new frame
 *  \param pd A peak detector
 *  \param frame An audio frame
 */
  
GAVL_PUBLIC
void gavl_peak_detector_update(gavl_peak_detector_t *pd,
                               gavl_audio_frame_t * frame);

/*! \brief Get the audio sink
 *  \param pd A peak detector
 *  \returns An audio sink
 *
 *  Use the returned sink for passing audio frames as an alternative to
 *  \ref gavl_peak_detector_update
 *
 *  Since 1.5.0
 */
  
GAVL_PUBLIC
gavl_audio_sink_t * gavl_peak_detector_get_sink(gavl_peak_detector_t *pd);
  
/*! \brief Get the peak volume across all channels
 *  \param pd A peak detector
 *  \param min Returns minimum amplitude
 *  \param max Returns maximum amplitude
 *  \param abs Returns maximum absolute amplitude
 *
 *  The returned amplitudes are normalized such that the
 *  minimum amplitude corresponds to -1.0, the maximum amplitude
 *  corresponds to 1.0.
 */
  
GAVL_PUBLIC
void gavl_peak_detector_get_peak(gavl_peak_detector_t * pd,
                                 double * min, double * max,
                                 double * abs);

/*! \brief Get the peak volume for all channels separate
 *  \param pd A peak detector
 *  \param min Returns minimum amplitude
 *  \param max Returns maximum amplitude
 *  \param abs Returns maximum absolute amplitude
 *
 *  The returned amplitudes are normalized such that the
 *  minimum amplitude corresponds to -1.0, the maximum amplitude
 *  corresponds to 1.0.
 */
  
GAVL_PUBLIC
void gavl_peak_detector_get_peaks(gavl_peak_detector_t * pd,
                                  double * min, double * max,
                                  double * abs);
  
/*! \brief Reset a peak detector
 *  \param pd A peak detector
 */
  
GAVL_PUBLIC
void gavl_peak_detector_reset(gavl_peak_detector_t * pd);

/**
 * @}
 */

  

#ifdef __cplusplus
}
#endif

#endif // GAVL_PEAKDETECTORS_H_INCLUDED
