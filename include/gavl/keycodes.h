/*****************************************************************
 * gmerlin - a general purpose multimedia framework and applications
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

/*
 *  System independent keycode definitions
 */

/** \defgroup keycodes System independent keycode definitions
 *  \ingroup msg
 *
 *  @{
 */

#define GAVL_KEY_SHIFT_MASK    (1<<0) //!< Shift
#define GAVL_KEY_CONTROL_MASK  (1<<1) //!< Control
#define GAVL_KEY_ALT_MASK      (1<<2) //!< Alt
#define GAVL_KEY_SUPER_MASK    (1<<3) //!< Windows key is called "Super" under X11
#define GAVL_KEY_BUTTON1_MASK  (1<<4) //!< Mouse button 1
#define GAVL_KEY_BUTTON2_MASK  (1<<5) //!< Mouse button 2
#define GAVL_KEY_BUTTON3_MASK  (1<<6) //!< Mouse button 3
#define GAVL_KEY_BUTTON4_MASK  (1<<7) //!< Mouse button 4
#define GAVL_KEY_BUTTON5_MASK  (1<<8) //!< Mouse button 5

#define GAVL_KEY_NONE      -1 //!< Undefined

#define GAVL_KEY_0          0 //!< 0
#define GAVL_KEY_1          1 //!< 1
#define GAVL_KEY_2          2 //!< 2
#define GAVL_KEY_3          3 //!< 3
#define GAVL_KEY_4          4 //!< 4
#define GAVL_KEY_5          5 //!< 5
#define GAVL_KEY_6          6 //!< 6
#define GAVL_KEY_7          7 //!< 7
#define GAVL_KEY_8          8 //!< 8
#define GAVL_KEY_9          9 //!< 9

#define GAVL_KEY_SPACE      10 //!< Space
#define GAVL_KEY_RETURN     11 //!< Return (Enter)
#define GAVL_KEY_LEFT       12 //!< Left
#define GAVL_KEY_RIGHT      13 //!< Right
#define GAVL_KEY_UP         14 //!< Up
#define GAVL_KEY_DOWN       15 //!< Down
#define GAVL_KEY_PAGE_UP    16 //!< Page Up
#define GAVL_KEY_PAGE_DOWN  17 //!< Page Down
#define GAVL_KEY_HOME       18 //!< Home
#define GAVL_KEY_END        19 //!< End
#define GAVL_KEY_PLUS       20 //!< Plus
#define GAVL_KEY_MINUS      21 //!< Minus
#define GAVL_KEY_TAB        22 //!< Tab
#define GAVL_KEY_ESCAPE     23 //!< Esc
#define GAVL_KEY_MENU       24 //!< Menu key

#define GAVL_KEY_QUESTION   24 //!< ?
#define GAVL_KEY_EXCLAM     25 //!< !
#define GAVL_KEY_QUOTEDBL   26 //!< "
#define GAVL_KEY_DOLLAR     27 //!< $
#define GAVL_KEY_PERCENT    28 //!< %
#define GAVL_KEY_APMERSAND  29 //!< &
#define GAVL_KEY_SLASH      30 //!< /
#define GAVL_KEY_LEFTPAREN  31 //!< (
#define GAVL_KEY_RIGHTPAREN 32 //!< )
#define GAVL_KEY_EQUAL      33 //!< =
#define GAVL_KEY_BACKSLASH  34 //!< :-)
#define GAVL_KEY_BACKSPACE  35 //!< :-)

#define GAVL_KEY_A 101 //!< A
#define GAVL_KEY_B 102 //!< B
#define GAVL_KEY_C 103 //!< C
#define GAVL_KEY_D 104 //!< D
#define GAVL_KEY_E 105 //!< E
#define GAVL_KEY_F 106 //!< F
#define GAVL_KEY_G 107 //!< G
#define GAVL_KEY_H 108 //!< H
#define GAVL_KEY_I 109 //!< I
#define GAVL_KEY_J 110 //!< J
#define GAVL_KEY_K 111 //!< K
#define GAVL_KEY_L 112 //!< L
#define GAVL_KEY_M 113 //!< M
#define GAVL_KEY_N 114 //!< N
#define GAVL_KEY_O 115 //!< O
#define GAVL_KEY_P 116 //!< P
#define GAVL_KEY_Q 117 //!< Q
#define GAVL_KEY_R 118 //!< R
#define GAVL_KEY_S 119 //!< S
#define GAVL_KEY_T 120 //!< T
#define GAVL_KEY_U 121 //!< U
#define GAVL_KEY_V 122 //!< V
#define GAVL_KEY_W 123 //!< W
#define GAVL_KEY_X 124 //!< X
#define GAVL_KEY_Y 125 //!< Y
#define GAVL_KEY_Z 126 //!< Z

#define GAVL_KEY_a 201 //!< a
#define GAVL_KEY_b 202 //!< b
#define GAVL_KEY_c 203 //!< c
#define GAVL_KEY_d 204 //!< d
#define GAVL_KEY_e 205 //!< e
#define GAVL_KEY_f 206 //!< f
#define GAVL_KEY_g 207 //!< g
#define GAVL_KEY_h 208 //!< h
#define GAVL_KEY_i 209 //!< i
#define GAVL_KEY_j 210 //!< j
#define GAVL_KEY_k 211 //!< k
#define GAVL_KEY_l 212 //!< l
#define GAVL_KEY_m 213 //!< m
#define GAVL_KEY_n 214 //!< n
#define GAVL_KEY_o 215 //!< o
#define GAVL_KEY_p 216 //!< p
#define GAVL_KEY_q 217 //!< q
#define GAVL_KEY_r 218 //!< r
#define GAVL_KEY_s 219 //!< s
#define GAVL_KEY_t 220 //!< t
#define GAVL_KEY_u 221 //!< u
#define GAVL_KEY_v 222 //!< v
#define GAVL_KEY_w 223 //!< w
#define GAVL_KEY_x 224 //!< x
#define GAVL_KEY_y 225 //!< y
#define GAVL_KEY_z 226 //!< z

#define GAVL_KEY_F1  301 //!< F1
#define GAVL_KEY_F2  302 //!< F2
#define GAVL_KEY_F3  303 //!< F3
#define GAVL_KEY_F4  304 //!< F4
#define GAVL_KEY_F5  305 //!< F5
#define GAVL_KEY_F6  306 //!< F6
#define GAVL_KEY_F7  307 //!< F7
#define GAVL_KEY_F8  308 //!< F8
#define GAVL_KEY_F9  309 //!< F9
#define GAVL_KEY_F10 310 //!< F10
#define GAVL_KEY_F11 311 //!< F11
#define GAVL_KEY_F12 312 //!< F12

/* Multimedia keys */

#define GAVL_KEY_MUTE         401
#define GAVL_KEY_VOLUME_PLUS  402
#define GAVL_KEY_VOLUME_MINUS 403
#define GAVL_KEY_PLAY         404
#define GAVL_KEY_PREV         405
#define GAVL_KEY_NEXT         406
#define GAVL_KEY_STOP         407
#define GAVL_KEY_PAUSE        408
#define GAVL_KEY_FORWARD      409
#define GAVL_KEY_REWIND       410

/**  @}
 */
