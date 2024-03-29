/*
 * $Id: rxvt.h,v 1.163 2003/03/07 01:00:32 gcw Exp $
 */

#ifndef _RXVT_H_		/* include once only */
#define _RXVT_H_

#include "rxvtlib.h"

#include "feature.h"

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>

/*
 *****************************************************************************
 * SYSTEM HACKS
 *****************************************************************************
 */
/* Consistent defines - please report on the necessity
 * @ Unixware: defines (__svr4__)
 */
#if defined (SVR4) && !defined (__svr4__)
# define __svr4__
#endif
#if defined (sun) && !defined (__sun__)
# define __sun__
#endif


#ifndef HAVE_XPOINTER
typedef char   *XPointer;
#endif

#ifdef HAVE_TERMIOS_H
# include <termios.h>
typedef struct termios ttymode_t;
#else
# include <sgtty.h>
typedef struct {
    struct sgttyb   sg;
    struct tchars   tc;
    struct ltchars  lc;
    int             line;
    int             local;
} ttymode_t;
#endif

#ifdef GREEK_SUPPORT
# include "grkelot.h"
#endif
#ifdef XPM_BACKGROUND
# ifdef XPM_INC_X11
#  include <X11/xpm.h>
# else
#  include <xpm.h>
# endif
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO	0
# define STDOUT_FILENO	1
# define STDERR_FILENO	2
#endif

#if defined(HAVE_GRANTPT) && defined(HAVE_UNLOCKPT)
# if defined(PTYS_ARE_GETPT) || defined(PTYS_ARE_PTMX)
#  define NO_SETOWNER_TTYDEV
# endif
#endif
#if defined(__CYGWIN32__) || defined(PTYS_ARE_OPENPTY)
# define NO_SETOWNER_TTYDEV
#endif

/*
 *****************************************************************************
 * STRUCTURES AND TYPEDEFS
 *****************************************************************************
 */
struct rxvt_vars;		/* Later REDEFINED and typedef'd to rxvt_t */
struct rxvt_hidden;
struct grwin_t;


/* Sanitize menubar info */
#ifndef MENUBAR
# undef MENUBAR_MAX
#endif
#ifndef MENUBAR_MAX
# define MENUBAR_MAX	0
#endif

/* If we're using either the rxvt scrollbar or menu bars, keep the
 * scrollColor resource.
 */
#if defined(RXVT_SCROLLBAR) || defined(MENUBAR)
# define KEEP_SCROLLCOLOR 1
#else
# undef KEEP_SCROLLCOLOR
#endif

#ifdef XPM_BACKGROUND
typedef struct {
    short           w, h, x, y;
    Pixmap          pixmap;
} bgPixmap_t;
#endif

/*
 * the 'essential' information for reporting Mouse Events
 * pared down from XButtonEvent
 */
struct mouse_event {
    int             clicks;
    Time            time;	/* milliseconds */
    unsigned int    state;	/* key or button mask */
    unsigned int    button;	/* detail */
};

#ifndef min
# define min(a,b)	(((a) < (b)) ? (a) : (b))
# define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#define MAX_IT(current, other)	if ((other) > (current)) (current) = (other)
#define MIN_IT(current, other)	if ((other) < (current)) (current) = (other)
#define SWAP_IT(one, two, typeof)					\
    do {								\
	typeof          swapittmp;					\
	(swapittmp) = (one); (one) = (two); (two) = (swapittmp);	\
    } while (/* CONSTCOND */ 0)
#define BOUND_POSITIVE_INT16(val)			\
    (int16_t)((val) <= 0				\
	      ? 0					\
	      : min((val), (((u_int16_t)-1)>>1)))

/*
 *****************************************************************************
 * NORMAL DEFINES
 *****************************************************************************
 */

#if defined (NO_OLD_SELECTION) && defined(NO_NEW_SELECTION)
# error if you disable both selection styles, how can you select, silly?
#endif

#define APL_CLASS	"XTerm"	/* class name */
#define APL_SUBCLASS	"Rxvt"	/* also check resources under this name */
#define APL_NAME	"rxvt"	/* normal name */

/* COLORTERM, TERM environment variables */
#define COLORTERMENV	"rxvt"
#ifdef XPM_BACKGROUND
# define COLORTERMENVFULL COLORTERMENV "-xpm"
#else
# define COLORTERMENVFULL COLORTERMENV
#endif
#ifndef TERMENV
# define TERMENV	"xterm"
#endif

#if defined (NO_MOUSE_REPORT) && !defined (NO_MOUSE_REPORT_SCROLLBAR)
# define NO_MOUSE_REPORT_SCROLLBAR
#endif

#ifdef NO_RESOURCES
# undef USE_XGETDEFAULT
#endif

/* now look for other badly set stuff */

#if !defined (EACCESS) && defined(EAGAIN)
# define EACCESS EAGAIN
#endif

#ifndef EXIT_SUCCESS		/* missing from <stdlib.h> */
# define EXIT_SUCCESS		0	/* exit function success */
# define EXIT_FAILURE		1	/* exit function failure */
#endif

#define menuBar_esc		10
#define scrollBar_esc		30
#define menuBar_margin		2	/* margin below text */

#if defined(RXVT_SCROLLBAR) || defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR)
# define HAVE_SCROLLBARS
#endif

/* width of scrollBar, menuBar shadow, must be 1 or 2 */
#ifdef HALFSHADOW
# define SHADOW 1
#else
# define SHADOW 2
#endif

#define R_SB_ALIGN_CENTRE	0
#define R_SB_ALIGN_TOP		1
#define R_SB_ALIGN_BOTTOM	2

#define R_SB_RXVT		0
#define R_SB_NEXT		1
#define R_SB_XTERM		2

#define SB_WIDTH_NEXT		19
#define SB_WIDTH_XTERM		15
#ifndef SB_WIDTH_RXVT
# define SB_WIDTH_RXVT		10
#endif

/*
 * NeXT scrollbar defines
 */
#define SB_PADDING		1
#define SB_BORDER_WIDTH		1
#define SB_BEVEL_WIDTH_UPPER_LEFT	1
#define SB_BEVEL_WIDTH_LOWER_RIGHT	2
#define SB_LEFT_PADDING		(SB_PADDING + SB_BORDER_WIDTH)
#define SB_MARGIN_SPACE		(SB_PADDING * 2)
#define SB_BUTTON_WIDTH		(SB_WIDTH_NEXT - SB_MARGIN_SPACE - SB_BORDER_WIDTH)
#define SB_BUTTON_HEIGHT	(SB_BUTTON_WIDTH)
#define SB_BUTTON_SINGLE_HEIGHT	(SB_BUTTON_HEIGHT + SB_PADDING)
#define SB_BUTTON_BOTH_HEIGHT	(SB_BUTTON_SINGLE_HEIGHT * 2)
#define SB_BUTTON_TOTAL_HEIGHT	(SB_BUTTON_BOTH_HEIGHT + SB_PADDING)
#define SB_BUTTON_BEVEL_X	(SB_LEFT_PADDING)
#define SB_BUTTON_FACE_X	(SB_BUTTON_BEVEL_X + SB_BEVEL_WIDTH_UPPER_LEFT)
#define SB_THUMB_MIN_HEIGHT	(SB_BUTTON_WIDTH - (SB_PADDING * 2))
 /*
  *    +-------------+
  *    |             | <---< SB_PADDING
  *    | ::::::::::: |
  *    | ::::::::::: |
  *   '''''''''''''''''
  *   ,,,,,,,,,,,,,,,,,
  *    | ::::::::::: |
  *    | ::::::::::: |
  *    |  +---------------< SB_BEVEL_WIDTH_UPPER_LEFT
  *    |  | :::::::: |
  *    |  V :::: vv-------< SB_BEVEL_WIDTH_LOWER_RIGHT
  *    | +---------+ |
  *    | | ......%%| |
  *    | | ......%%| |
  *    | | ..()..%%| |
  *    | | ......%%| |
  *    | | %%%%%%%%| |
  *    | +---------+ | <.........................
  *    |             | <---< SB_PADDING         :
  *    | +---------+ | <-+..........            :---< SB_BUTTON_TOTAL_HEIGHT
  *    | | ......%%| |   |         :            :
  *    | | ../\..%%| |   |---< SB_BUTTON_HEIGHT :
  *    | | %%%%%%%%| |   |         :            :
  *    | +---------+ | <-+         :            :
  *    |             |             :            :
  *    | +---------+ | <-+         :---< SB_BUTTON_BOTH_HEIGHT
  *    | | ......%%| |   |         :            :
  *    | | ..\/..%%| |   |         :            :
  *    | | %%%%%%%%| |   |---< SB_BUTTON_SINGLE_HEIGHT
  *    | +---------+ |   |         :            :
  *    |             |   |         :            :
  *    +-------------+ <-+.........:............:
  *    ^^|_________| :
  *    ||     |      :
  *    ||     +---< SB_BUTTON_WIDTH
  *    ||            :
  *    |+------< SB_PADDING
  *    |:            :
  *    +----< SB_BORDER_WIDTH
  *     :            :
  *     :............:
  *           |
  *           +---< SB_WIDTH_NEXT
  */

#define NO_REFRESH		0	/* Window not visible at all!        */
#define FAST_REFRESH		(1<<0)	/* Fully exposed window              */
#define SLOW_REFRESH		(1<<1)	/* Partially exposed window          */
#define SMOOTH_REFRESH		(1<<2)	/* Do sync'ing to make it smooth     */
#define REFRESH_BOUNDS		(1<<3)

#ifdef NO_SECONDARY_SCREEN
# define NSCREENS		0
#else
# define NSCREENS		1
#endif

#define IGNORE			0
#define SAVE			's'
#define RESTORE			'r'

/* special (internal) prefix for font commands */
#define FONT_CMD		'#'
#define FONT_DN			"#-"
#define FONT_UP			"#+"

/* flags for rxvt_scr_gotorc() */
#define C_RELATIVE		1	/* col movement is relative */
#define R_RELATIVE		2	/* row movement is relative */
#define RELATIVE		(R_RELATIVE|C_RELATIVE)

/* modes for rxvt_scr_insdel_chars(), rxvt_scr_insdel_lines() */
#define INSERT			-1	/* don't change these values */
#define DELETE			+1
#define ERASE			+2

/* modes for rxvt_scr_page() - scroll page. used by scrollbar window */
enum page_dirn {
    UP,
    DN,
    NO_DIR
};

/* arguments for rxvt_scr_change_screen() */
enum {
    PRIMARY = 0,
    SECONDARY
};

enum {
    SBYTE = 0,
    WBYTE
};


#define RS_None			0	/* Normal */

#if defined(TTY_256COLOR)
/* have at least 32 bits to use */
# define RS_fgMask		0x000001FFu	/* 512 colors */
# define RS_bgMask		0x0003FE00u	/* 512 colors */
# define RS_Bold		0x00040000u	/* bold */
# define RS_Blink		0x00080000u	/* blink */
# define RS_RVid		0x00100000u	/* reverse video */
# define RS_Uline		0x00200000u	/* underline */
# define RS_acsFont		0x00400000u	/* ACS graphics char set */
# define RS_ukFont		0x00800000u	/* UK character set */
#else
/* may only have 16 bits to use so squash them in */
# define RS_fgMask		0x0000001Fu	/* 32 colors */
# define RS_bgMask		0x000003E0u	/* 32 colors */
# define RS_Bold		0x00000400u	/* bold */
# define RS_Blink		0x00000800u	/* blink */
# define RS_RVid		0x00001000u	/* reverse video */
# define RS_Uline		0x00002000u	/* underline */
# define RS_acsFont		0x00004000u	/* ACS graphics char set */
# define RS_ukFont		0x00008000u	/* UK character set */
#endif

#ifdef MULTICHAR_SET
# define RS_multi0		0x10000000u	/* only multibyte characters */
# define RS_multi1		0x20000000u	/* multibyte 1st byte */
# define RS_multi2		(RS_multi0|RS_multi1)	/* multibyte 2nd byte */
# define RS_multiMask		(RS_multi0|RS_multi1)	/* multibyte mask */
# define IS_MULTI1(r)		(((r) & RS_multiMask) == RS_multi1)
# define IS_MULTI2(r)		(((r) & RS_multiMask) == RS_multi2)
#else
# define RS_multiMask		0
# define IS_MULTI1(r)		(0)
# define IS_MULTI2(r)		(0)
#endif

#define RS_fontMask		(RS_acsFont|RS_ukFont)
#define RS_baseattrMask		(RS_Bold|RS_Blink|RS_RVid|RS_Uline)
#define RS_attrMask		(RS_baseattrMask|RS_fontMask|RS_multiMask)

#define Sel_none		0	/* Not waiting */
#define Sel_normal		0x01	/* normal selection */
#define Sel_incr		0x02	/* incremental selection */
#define Sel_direct		0x00
#define Sel_Primary		0x01
#define Sel_Secondary		0x02
#define Sel_Clipboard		0x03
#define Sel_whereMask		0x0f
#define Sel_CompoundText	0x10	/* last request was Compound */

enum {
    C0_NUL = 0x00,
            C0_SOH, C0_STX, C0_ETX, C0_EOT, C0_ENQ, C0_ACK, C0_BEL,
    C0_BS , C0_HT , C0_LF , C0_VT , C0_FF , C0_CR , C0_SO , C0_SI ,
    C0_DLE, C0_DC1, C0_DC2, D0_DC3, C0_DC4, C0_NAK, C0_SYN, C0_ETB,
    C0_CAN, C0_EM , C0_SUB, C0_ESC, C0_IS4, C0_IS3, C0_IS2, C0_IS1
}; 
#define CHAR_ST			0x9c	/* 0234 */

/*
 * XTerm Operating System Commands: ESC ] Ps;Pt (ST|BEL)
 * colour extensions by Christian W. Zuckschwerdt <zany@triq.net>
 */
#define XTerm_name		0
#define XTerm_iconName		1
#define XTerm_title		2
#define XTerm_Color		4	/* change colors */
#define XTerm_Color_cursor	12	/* change actual 'Cursor' color */
#define XTerm_Color_pointer	13	/* change actual 'Pointer' color */
#define XTerm_Color_RV		17	/* change actual 'Highlight' color */
#define XTerm_Color_BD		18	/* change actual 'Bold' color */
#define XTerm_Color_UL		19	/* change actual 'Underline' color */
#define XTerm_logfile		46	/* not implemented */
#define XTerm_font		50

/*
 * rxvt extensions of XTerm OSCs: ESC ] Ps;Pt (ST|BEL)
 */
#define XTerm_Menu		10	/* set menu item */
#define XTerm_Pixmap		20	/* new bg pixmap */
#define XTerm_restoreFG		39	/* change default fg color */
#define XTerm_restoreBG		49	/* change default bg color */
#define XTerm_dumpscreen	55	/* dump scrollback and all of screen */

/* Words starting with `Color_' are colours.  Others are counts */
/*
 * The following comment is mostly obsolete since pixcolor_set was expanded:
 * We're currently upto 29 colours.  Only 3 more available.  The
 * PixColor and rendition colour usage should probably be decoupled
 * on the unnecessary items, e.g. Color_pointer, but won't bother
 * until we need to.  Also, be aware of usage in pixcolor_set
 */

enum colour_list {
    Color_fg = 0,
    Color_bg,
    minCOLOR,			/* 2 */
    Color_Black = minCOLOR,
    Color_Red3,
    Color_Green3,
    Color_Yellow3,
    Color_Blue3,
    Color_Magenta3,
    Color_Cyan3,
    maxCOLOR,			/* minCOLOR + 7 */
#ifndef NO_BRIGHTCOLOR
    Color_AntiqueWhite = maxCOLOR,
    minBrightCOLOR,		/* maxCOLOR + 1 */
    Color_Grey25 = minBrightCOLOR,
    Color_Red,
    Color_Green,
    Color_Yellow,
    Color_Blue,
    Color_Magenta,
    Color_Cyan,
    maxBrightCOLOR,		/* minBrightCOLOR + 7 */
    Color_White = maxBrightCOLOR,
#else
    Color_White = maxCOLOR,
#endif
#ifdef TTY_256COLOR
    min256COLOR = Color_White + 1,
    max256COLOR = minCOLOR + 255,
#endif
#ifndef NO_CURSORCOLOR
    Color_cursor,
    Color_cursor2,
#endif
    Color_pointer,
    Color_border,
#ifndef NO_BOLD_UNDERLINE_REVERSE
    Color_BD,
    Color_UL,
    Color_RV,
#endif
#ifdef OPTION_HC
    Color_HC,
#endif
#ifdef KEEP_SCROLLCOLOR
    Color_scroll,
    Color_trough,
#endif
    NRS_COLORS,			/* */
#ifdef KEEP_SCROLLCOLOR
    Color_topShadow = NRS_COLORS,
    Color_bottomShadow,
    TOTAL_COLORS		/* upto 30 */
#else
    TOTAL_COLORS = NRS_COLORS	/* */
#endif
};

#ifdef TTY_256COLOR
# define Color_Bits	9
# define NPIXCLR_SETS	9	/* (256 + 14) bits / 32 bits */
#else
# define Color_Bits	5
# define NPIXCLR_SETS	1	/* (16 + 14) bits / 32 bits */
#endif
#define NPIXCLR_BITS	32

#define DEFAULT_RSTYLE		(RS_None | (Color_fg) | (Color_bg<<Color_Bits))

/*
 * Resource list
 */
enum {
    Rs_display_name = 0,
    Rs_term_name,
    Rs_iconName,
    Rs_geometry,
    Rs_reverseVideo,
    Rs_color,
    _Rs_color = Rs_color + NRS_COLORS - 1,
    Rs_font,
    _Rs_font = Rs_font + MAX_NFONTS - 1,
#ifdef MULTICHAR_SET
    Rs_mfont,
    _Rs_mfont = Rs_mfont + MAX_NFONTS - 1,
    Rs_multichar_encoding,
    Rs_mc_hack,
#endif
    Rs_name,
    Rs_title,
#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
    Rs_path,
#endif
#ifdef XPM_BACKGROUND
    Rs_backgroundPixmap,
#endif
#if (MENUBAR_MAX)
    Rs_menu,
#endif
#ifndef NO_BOLDFONT
    Rs_boldFont,
#endif
#ifdef GREEK_SUPPORT
    Rs_greek_keyboard,
    Rs_greektoggle_key,
#endif
    Rs_loginShell,
    Rs_jumpScroll,
#ifdef HAVE_SCROLLBARS
    Rs_scrollBar,
    Rs_scrollBar_right,
    Rs_scrollBar_floating,
    Rs_scrollBar_align,
#endif
    Rs_scrollstyle,	/* Rs_scrollBar_style */
    Rs_scrollTtyOutput,
    Rs_scrollTtyKeypress,
    Rs_scrollWithBuffer,
    Rs_saveLines,
    Rs_utmpInhibit,
    Rs_visualBell,
#if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
    Rs_mapAlert,
#endif
#ifdef META8_OPTION
    Rs_meta8,
#endif
#ifdef MOUSE_WHEEL
    Rs_mouseWheelScrollPage,
#endif
#ifndef NO_BACKSPACE_KEY
    Rs_backspace_key,
#endif
#ifndef NO_DELETE_KEY
    Rs_delete_key,
#endif
    Rs_selectstyle,
#ifdef PRINTPIPE
    Rs_print_pipe,
#endif
#ifdef USE_XIM
    Rs_preeditType,
    Rs_inputMethod,
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
    Rs_bigfont_key,
    Rs_smallfont_key,
#endif
#ifdef TRANSPARENT
    Rs_transparent,
    Rs_transparent_all,
#endif
#ifndef NO_FRILLS
    Rs_ext_bwidth,
    Rs_int_bwidth,
#endif
    Rs_scrollBar_thickness,
#ifndef NO_LINESPACE
    Rs_lineSpace,
#endif
    Rs_cutchars,
    Rs_modifier,
    Rs_answerbackstring,
    Rs_tripleclickwords,
    NUM_RESOURCES
} ;

enum {
    TIMEOUT_INCR = 0,
    NUM_TIMEOUTS
} ;

enum {
    XA_COMPOUND_TEXT = 0,
    XA_MULTIPLE,
    XA_TARGETS,
    XA_TEXT,
    XA_TIMESTAMP,
    XA_VT_SELECTION,
    XA_INCR,
    XA_WMDELETEWINDOW,
#ifdef TRANSPARENT
    XA_XROOTPMAPID,
#endif
#ifdef OFFIX_DND		/* OffiX Dnd (drag 'n' drop) support */
    XA_DNDPROTOCOL,
    XA_DNDSELECTION,
#endif				/* OFFIX_DND */
    XA_CLIPBOARD,
    NUM_XA
} ;

/*
 * number of graphics points
 * divisible by 2 (num lines)
 * divisible by 4 (num rect)
 */
#define	NGRX_PTS	1000

/* DEC private modes */
#define PrivMode_132		(1LU<<0)
#define PrivMode_132OK		(1LU<<1)
#define PrivMode_rVideo		(1LU<<2)
#define PrivMode_relOrigin	(1LU<<3)
#define PrivMode_Screen		(1LU<<4)
#define PrivMode_Autowrap	(1LU<<5)
#define PrivMode_aplCUR		(1LU<<6)
#define PrivMode_aplKP		(1LU<<7)
#define PrivMode_HaveBackSpace	(1LU<<8)
#define PrivMode_BackSpace	(1LU<<9)
#define PrivMode_ShiftKeys	(1LU<<10)
#define PrivMode_VisibleCursor	(1LU<<11)
#define PrivMode_MouseX10	(1LU<<12)
#define PrivMode_MouseX11	(1LU<<13)
#define PrivMode_scrollBar	(1LU<<14)
#define PrivMode_menuBar	(1LU<<15)
#define PrivMode_TtyOutputInh	(1LU<<16)
#define PrivMode_Keypress	(1LU<<17)
#define PrivMode_smoothScroll	(1LU<<18)
#define PrivMode_vt52		(1LU<<19)
/* too annoying to implement X11 highlight tracking */
/* #define PrivMode_MouseX11Track       (1LU<<18) */

#define PrivMode_mouse_report	(PrivMode_MouseX10|PrivMode_MouseX11)
#define PrivMode(test,bit)		\
    if (test)				\
	r->h->PrivateModes |= (bit);	\
    else				\
	r->h->PrivateModes &= ~(bit)

#ifdef ALLOW_132_MODE
# define PrivMode_Default						 \
(PrivMode_Autowrap|PrivMode_aplKP|PrivMode_ShiftKeys|PrivMode_VisibleCursor|PrivMode_132OK)
#else
# define PrivMode_Default						 \
(PrivMode_Autowrap|PrivMode_aplKP|PrivMode_ShiftKeys|PrivMode_VisibleCursor)
#endif

#ifdef PREFER_24BIT
# define XDEPTH			r->Xdepth
# define XCMAP			r->Xcmap
# define XVISUAL		r->h->Xvisual
#else
# ifdef DEBUG_DEPTH
#  define XDEPTH		DEBUG_DEPTH
# else
#  define XDEPTH		DefaultDepth(r->Xdisplay,Xscreen)
#  define XCMAP			DefaultColormap(r->Xdisplay,Xscreen)
#  define XVISUAL		DefaultVisual(r->Xdisplay,Xscreen)
# endif
#endif
#define IMBUFSIZ		128	/* input modifier buffer sizes */
#ifndef BUFSIZ
# define BUFSIZ			4096
#endif
#define KBUFSZ			512	/* size of keyboard mapping buffer */

/*
 *****************************************************************************
 * MACRO DEFINES
 *****************************************************************************
 */
#define MEMSET(x, y, z)		memset((x), (y), (size_t)(z))
#define MEMCPY(x, y, z)		memcpy((void *)(x), (const void *)(y), (z))
#define MEMMOVE(x, y, z)	memmove((void *)(x), (const void *)(y), (z))
#define STRCASECMP(x, y)	strcasecmp((x), (y))
#define STRNCASECMP(x, y, z)	strncasecmp((x), (y), (z))
#define STRCPY(x, y)		strcpy((char *)(x), (const char *)(y))
#define STRNCPY(x, y, z)	strncpy((char *)(x), (const char *)(y), (z))
#define STRCMP(x, y)		strcmp((const char *)(x), (const char *)(y))
#define STRNCMP(x, y, z)	strncmp((const char *)(x), (const char *)(y), (z))
#define STRCAT(x, y)		strcat((char *)(x), (const char *)(y))
#define STRNCAT(x, y, z)	strncat((char *)(x), (const char *)(y), (z))
#define STRDUP(x)		strdup((const char *)(x))
#define STRLEN(x)		strlen((const char *)(x))
#define STRCHR(x, y)		strchr((const char *)(x), (int)(y))
#define STRRCHR(x, y)		strrchr((const char *)(x), (int)(y))

/* convert pixel dimensions to row/column values.  Everything as int32_t */
#define Pixel2Col(x)		Pixel2Width((int32_t)(x) - (int32_t)r->TermWin.int_bwidth)
#define Pixel2Row(y)		Pixel2Height((int32_t)(y) - (int32_t)r->TermWin.int_bwidth)
#define Pixel2Width(x)		((int32_t)(x) / (int32_t)r->TermWin.fwidth)
#define Pixel2Height(y)		((int32_t)(y) / (int32_t)r->TermWin.fheight)
#define Col2Pixel(col)		((int32_t)Width2Pixel(col) + (int32_t)r->TermWin.int_bwidth)
#define Row2Pixel(row)		((int32_t)Height2Pixel(row) + (int32_t)r->TermWin.int_bwidth)
#define Width2Pixel(n)		((int32_t)(n) * (int32_t)r->TermWin.fwidth)
#define Height2Pixel(n)		((int32_t)(n) * (int32_t)r->TermWin.fheight)

#define TermWin_TotalWidth()	((int32_t)r->TermWin.width  + 2 * (int32_t)r->TermWin.int_bwidth)
#define TermWin_TotalHeight()	((int32_t)r->TermWin.height + 2 * (int32_t)r->TermWin.int_bwidth)

#define Xscreen			DefaultScreen(r->Xdisplay)
#define Xroot			DefaultRootWindow(r->Xdisplay)

/* how to build & extract colors and attributes */
#define GET_BASEFG(x)		(((x) & RS_fgMask))
#define GET_BASEBG(x)		(((x) & RS_bgMask)>>Color_Bits)
#ifndef NO_BRIGHTCOLOR
# define GET_FGCOLOR(x)						\
    ((((x) & RS_Bold) == 0					\
      || GET_BASEFG(x) < minCOLOR				\
      || GET_BASEFG(x) >= minBrightCOLOR)			\
     ? GET_BASEFG(x)						\
     : (GET_BASEFG(x) + (minBrightCOLOR - minCOLOR)))
# define GET_BGCOLOR(x)						\
    ((((x) & RS_Blink) == 0					\
      || GET_BASEBG(x) < minCOLOR				\
      || GET_BASEBG(x) >= minBrightCOLOR)			\
     ? GET_BASEBG(x)						\
     : (GET_BASEBG(x) + (minBrightCOLOR - minCOLOR)))
#else
# define GET_FGCOLOR(x)		GET_BASEFG(x)
# define GET_BGCOLOR(x)		GET_BASEBG(x)
#endif

#define GET_ATTR(x)		(((x) & RS_attrMask))
#define GET_BGATTR(x)							\
    (((x) & RS_RVid) ? (((x) & (RS_attrMask & ~RS_RVid))		\
			| (((x) & RS_fgMask)<<Color_Bits))		\
		     : ((x) & (RS_attrMask | RS_bgMask)))
#define SET_FGCOLOR(x,fg)	(((x) & ~RS_fgMask)  | (fg))
#define SET_BGCOLOR(x,bg)	(((x) & ~RS_bgMask)  | ((bg)<<Color_Bits))
#define SET_ATTR(x,a)		(((x) & ~RS_attrMask)| (a))

#define SET_PIXCOLOR(h, x)	((h)->pixcolor_set[(x) / NPIXCLR_BITS] |= (1 << ((x) % NPIXCLR_BITS)))
#define ISSET_PIXCOLOR(h, x)	((h)->pixcolor_set[(x) / NPIXCLR_BITS] & (1 << ((x) % NPIXCLR_BITS)))

#ifdef HAVE_SCROLLBARS
# define scrollbar_TotalWidth()	(r->scrollBar.width + r->sb_shadow * 2)
#else
# define scrollbar_TotalWidth()	(0)
#endif
#define scrollbar_isMotion()	(r->scrollBar.state == 'm')
#define scrollbar_isUp()	(r->scrollBar.state == 'U')
#define scrollbar_isDn()	(r->scrollBar.state == 'D')
#define scrollbar_isUpDn()	isupper (r->scrollBar.state)
#define isScrollbarWindow(w)	(r->scrollBar.state && (w) == r->scrollBar.win)

#define scrollbar_setIdle()	r->scrollBar.state = 1
#define scrollbar_setMotion()	r->scrollBar.state = 'm'
#define scrollbar_setUp()	r->scrollBar.state = 'U'
#define scrollbar_setDn()	r->scrollBar.state = 'D'

#define scrollbarnext_dnval()	(r->scrollBar.end + (r->scrollBar.width + 1))
#define scrollbarnext_upButton(y)	((y) > r->scrollBar.end \
					 && (y) <= scrollbarnext_dnval())
#define scrollbarnext_dnButton(y)	((y) > scrollbarnext_dnval())
#define SCROLLNEXT_MINHEIGHT	SB_THUMB_MIN_HEIGHT
#define scrollbarrxvt_upButton(y)	((y) < r->scrollBar.beg)
#define scrollbarrxvt_dnButton(y)	((y) > r->scrollBar.end)
#define SCROLLRXVT_MINHEIGHT	10
#define SCROLLXTERM_MINHEIGHT	10

#define scrollbar_minheight()	(r->scrollBar.style == R_SB_NEXT	\
				 ? SCROLLNEXT_MINHEIGHT			\
				 : SCROLLRXVT_MINHEIGHT)
#define scrollbar_above_slider(y)	((y) < r->scrollBar.top)
#define scrollbar_below_slider(y)	((y) > r->scrollBar.bot)
#define scrollbar_position(y)		((y) - r->scrollBar.beg)
#define scrollbar_size()		(r->scrollBar.end - r->scrollBar.beg \
					 - scrollbar_minheight())

#if (MENUBAR_MAX > 1)
/* rendition style flags */
# define menuBar_height()	(r->TermWin.fheight + SHADOW)
# define menuBar_TotalHeight()	(menuBar_height() + SHADOW + menuBar_margin)
# define isMenuBarWindow(w)	((w) == r->menuBar.win)
#else
# define menuBar_height()	(0)
# define menuBar_TotalHeight()	(0)
# define isMenuBarWindow(w)	(0)
#endif

#ifdef XPM_BACKGROUND
# define XPMClearArea(a, b, c, d, e, f, g)	XClearArea((a), (b), (c), (d), (e), (f), (g))
#else
# define XPMClearArea(a, b, c, d, e, f, g)
#endif

#ifndef STRICT_FONT_CHECKING
# define rxvt_get_fontwidest(font)	((font)->max_bounds.width)
#endif

#define rxvt_Gr_ButtonPress(x,y)	rxvt_Gr_ButtonReport (r, 'P',(x),(y))
#define rxvt_Gr_ButtonRelease(x,y)	rxvt_Gr_ButtonReport (r, 'R',(x),(y))

#ifdef UTMP_SUPPORT
# if !defined(RXVT_UTMPX_FILE) || !defined(HAVE_STRUCT_UTMPX)
#  undef HAVE_UTMPX_H
#  undef HAVE_STRUCT_UTMPX
# endif
# if !defined(RXVT_UTMP_FILE) || !defined(HAVE_STRUCT_UTMP)
#  undef HAVE_UTMP_H
#  undef HAVE_STRUCT_UTMP
# endif

# ifdef HAVE_UTMPX_H
#  include <utmpx.h>
# endif
# ifdef HAVE_UTMP_H
#  include <utmp.h>
# endif
#endif

#ifdef DEBUG_CMD
# define D_CMD(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_CMD(x)
#endif
#ifdef DEBUG_INIT
# define D_INIT(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_INIT(x)
#endif
#ifdef DEBUG_MAIN
# define D_MAIN(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_MAIN(x)
#endif
#ifdef DEBUG_SCREEN
# define D_SCREEN(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_SCREEN(x)
#endif
#ifdef DEBUG_SELECT
# define D_SELECT(x)		fprintf x ; fputc('\n', stderr)
#else
# define D_SELECT(x)
#endif
#ifdef DEBUG_X
# define D_X(x)			fprintf x ; fputc('\n', stderr)
#else
# define D_X(x)
#endif

/*
 *****************************************************************************
 * VARIABLES
 *****************************************************************************
 */
#ifdef MENUBAR
# include "menubar.h"
#endif

struct rxvt_hidden {
#ifdef __GNUC__
    unsigned char   want_refresh:1,
                    want_full_refresh:1,
                    am_transparent:1,
                    am_pixmap_trans:1, 
                    current_screen:1,
                    hate_those_clicks:1,
                    num_scr_allow:1,
                    bypass_keystate:1;
    unsigned char   chstat:1,
                    lost_multi:1,
                    multi_byte:1,
                    parsed_geometry:1;
#else
    unsigned char   want_refresh,
# ifdef TRANSPARENT
                    want_full_refresh,	/* awaiting full screen refresh      */
# endif
# if defined(XPM_BACKGROUND) || defined(TRANSPARENT)
                    am_transparent,	/* is a transparent term             */
                    am_pixmap_trans,	/* transparency w/known root pixmap  */
# endif
                    current_screen,	/* primary or secondary              */
                    hate_those_clicks,	/* a.k.a. keep mark position         */
                    num_scr_allow,
                    bypass_keystate,
# ifdef MULTICHAR_SET
                    chstat,
                    lost_multi,	/* set ==> we only got half a glyph */
                    multi_byte,	/* set ==> currently using 2 bytes per glyph */
# endif
                    parsed_geometry;
#endif	/* !__GNUC__ */

    unsigned char   refresh_type,
#ifdef UTMP_SUPPORT
                    next_utmp_action,
#endif
#ifndef NO_SETOWNER_TTYDEV
                    next_tty_action,
#endif
#ifdef META8_OPTION
                    meta_char,	/* Alt-key prefix                            */
#endif
		    scrollbar_align,
                    selection_wait,
                    selection_type;
/* ---------- */
#ifdef GREEK_SUPPORT
    short           greek_mode;		/* greek keyboard mode               */
#endif
    short           rvideo;
    int16_t         num_scr;	/* screen: number lines scrolled             */
    u_int16_t       prev_ncol,	/* screen: previous number of columns        */
                    prev_nrow;	/* screen: previous number of rows           */
#ifdef RXVT_GRAPHICS
    u_int16_t       gr_prev_start;
#endif
/* ---------- */
    rend_t          rstyle;
/* ---------- */
    u_int32_t       pixcolor_set[NPIXCLR_SETS];
/* ---------- */
    int             csrO,	/* Hops - csr offset in thumb/slider to      */
				/*   give proper Scroll behaviour            */
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
                    scroll_arrow_delay,
#endif
#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
		    mouse_slip_wheel_delay,
		    mouse_slip_wheel_speed,
#endif
                    refresh_count,
                    refresh_limit,
                    fnum,	/* logical font number                       */
                    last_bot,	/* scrollbar last bottom position            */
                    last_top,	/* scrollbar last top position               */
                    last_state,	/* scrollbar last state                      */
                    scrollbar_len,
                    currmaxcol,
#ifdef MENUBAR
                    menu_readonly,	/* okay to alter menu? */
                    Arrows_x,
#endif
#if (MENUBAR_MAX > 1)
                    Nbars,
#endif
                    window_vt_x,
                    window_vt_y,
                    window_sb_x,
                    allowedxerror;
/* ---------- */
    unsigned int    ModMetaMask,
                    ModNumLockMask,
                    old_width,	/* last used width in screen resize          */
                    old_height,	/* last used height in screen resize         */
#ifndef NO_BRIGHTCOLOR
                    colorfgbg,
#endif
                    ttymode;
    unsigned long   PrivateModes,
                    SavedModes;
/* ---------- */
#ifdef PREFER_24BIT
    Visual         *Xvisual;
#endif
/* ---------- */
    Atom            xa[NUM_XA];
/* ---------- */
#ifdef MENUBAR
    GC              menubarGC;
#endif
#if defined(MENUBAR) || defined(RXVT_SCROLLBAR)
    GC              scrollbarGC,
                    topShadowGC,
                    botShadowGC;
#endif
#ifdef XTERM_SCROLLBAR
    GC              xscrollbarGC,
                    ShadowGC;
#endif
#ifdef NEXT_SCROLLBAR
    GC              blackGC,
                    whiteGC,
                    grayGC,
                    darkGC,
                    stippleGC;
    Pixmap          dimple,
                    upArrow,
                    downArrow,
                    upArrowHi,
                    downArrowHi;
#endif
/* ---------- */
    Time            selection_time;
    Time            selection_request_time;
    pid_t           cmd_pid;	/* process id of child */
    gid_t           ttygid;
#if (defined(HAVE_SETEUID) || defined(HAVE_SETREUID)) && !defined(__CYGWIN32__)
    uid_t           euid;
    gid_t           egid;
#endif
/* ---------- */
    Cursor          cursor_leftptr;
/* ---------- */
    const char     *ttydev;	/* pty/tty name */
#ifndef NO_BACKSPACE_KEY
    const char     *key_backspace;
#endif
#ifndef NO_DELETE_KEY
    const char     *key_delete;
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
    KeySym          ks_bigfont, ks_smallfont;
#endif
#ifdef GREEK_SUPPORT
    KeySym          ks_greekmodeswith;
#endif
#ifdef USE_XIM
    XIC             Input_Context;
    XIMStyle        input_style;
    int             event_type;
#endif
    struct mouse_event MEvent;
    XComposeStatus  compose;
#ifdef RXVT_GRAPHICS
    int             graphics_up;
    struct grwin_t *gr_root;
#endif
    ttymode_t       tio;
#ifdef UTMP_SUPPORT
# ifdef HAVE_STRUCT_UTMP
    struct utmp     ut;
# endif
# ifdef HAVE_STRUCT_UTMPX
    struct utmpx    utx;
# endif
# if (defined(HAVE_STRUCT_UTMP) && defined(HAVE_UTMP_PID)) || defined(HAVE_STRUCT_UTMPX)
    char            ut_id[5];
# endif
    int             utmp_pos;
#endif
    row_col_t       oldcursor;
#ifdef XPM_BACKGROUND
    bgPixmap_t      bgPixmap;
    XpmAttributes   xpmAttr;	/* originally loaded pixmap and its scaling */
#endif
#ifdef MULTICHAR_SET
    int             oldcursormulti;
    void            (*multichar_decode)(unsigned char *str, int len);
#endif
#ifndef RESET_TTY_TO_COMMON_DEFAULTS
    struct stat     ttyfd_stat;	/* original status of our tty */
#endif
#ifdef MENUBAR
    menu_t         *ActiveMenu,		/* currently active menu */
                   *BuildMenu;		/* the menu currently being built */
    bar_t          *CurrentBar;
# if !(MENUBAR_MAX > 1)
    bar_t           BarList;
# endif				/* (MENUBAR_MAX > 1) */
#endif
#ifdef RXVT_GRAPHICS
    Window          gr_last_id;
#endif
    struct timeval  timeout[NUM_TIMEOUTS];

/* these three don't need to be kept but do so to placate some mem checkers */
    char           *env_windowid;	/* environmental variable WINDOWID */
    char	   *env_display;	/* environmental variable DISPLAY  */
    char	   *env_term;		/* environmental variable TERM     */
    char           *env_colorfgbg;
    char           *buffer;
    char           *locale;
    char            charsets[4];
    unsigned char  *v_buffer;	/* pointer to physical buffer */
    unsigned char  *v_bufstr;	/* beginning of area to write */
    unsigned char  *v_bufptr;	/* end of area to write */
    unsigned char  *v_bufend;	/* end of physical buffer */
    char           *newfont[MAX_NFONTS];
#ifdef KEYSYM_RESOURCE
    const unsigned char *Keysym_map[256];
#endif
    const char     *rs[NUM_RESOURCES];
/* command input buffering */
    unsigned char  *cmdbuf_ptr, *cmdbuf_endp;
    unsigned char   cmdbuf_base[BUFSIZ];
    unsigned char   kbuf[KBUFSZ];
};

#ifndef __attribute__
# ifdef __GNUC__
#  if (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || (__GNUC__ < 2)
#   define __attribute__(x)
#  endif
# endif
# define __attribute__(x)
#endif

/*
 *****************************************************************************
 * PROTOTYPES
 *****************************************************************************
 */
#ifdef PROTOTYPES
# define __PROTO(p)	p
#else
# define __PROTO(p)	()
#endif
#include "protos.h"

#ifndef RXVT_GRAPHICS		/* sync functions with graphics.extpro */
# define rxvt_Gr_ButtonReport(r, but, x, y)
# define rxvt_Gr_do_graphics(r, cmd, nargs, args, text)
# define rxvt_Gr_scroll(r, count)
# define rxvt_Gr_ClearScreen(r)
# define rxvt_Gr_ChangeScreen(r)
# define rxvt_Gr_expose(r, win)
# define rxvt_Gr_Resize(r, w, h)
# define rxvt_Gr_reset(r)
# define rxvt_Gr_Displayed(r)		(0)
#endif

#ifndef MENUBAR			/* sync functions with menubar.extpro */
# define rxvt_menubar_read(r, filename)
# define rxvt_menubar_dispatch(r, str)
# define rxvt_menubar_expose(r)
# define rxvt_menubar_mapping(r, map)	(0)
# define rxvt_menubar_control(r, ev)
# define rxvt_map_menuBar(r, map)
# define rxvt_create_menuBar(r, cursor)
# define rxvt_Resize_menuBar(r, x, y, width, height)
#endif

#ifndef XPM_BACKGROUND		/* sync functions with xpm.extpro */
# define rxvt_scale_pixmap(r, geom)	(0)
# define rxvt_resize_pixmap(r)
# define rxvt_set_bgPixmap(r, file)	(0)
#endif

#ifndef GREEK_SUPPORT		/* sync functions with grkelot.extpro */
# define greek_init()
# define greek_end()
# define greek_reset()
# define greek_setmode(greek_mode)
# define greek_getmode()	(0)
# define greek_xlat(s, num_chars)	(0)
#endif

#ifndef USE_XIM
# define rxvt_setTermFontSet(r, idx)	(0)
#endif

#ifndef UTMP_SUPPORT
# define rxvt_privileged_utmp(r, action)	(0)
#endif

#ifdef NO_SETOWNER_TTYDEV
# define rxvt_privileged_ttydev(r, action)	(0)
#endif

#ifndef XTERM_COLOR_CHANGE
# define rxvt_set_window_color(r, idx, color)	(0)
#endif

#ifdef __CYGWIN32__
# define rxvt_privileged_ttydev(r, action)	(0)
#endif

#ifdef DEBUG_malloc
# include "dmalloc.h"		/* This comes last */
#endif

#endif				/* _RXVT_H_ */
