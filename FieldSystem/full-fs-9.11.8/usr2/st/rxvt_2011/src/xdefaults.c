/*--------------------------------*-C-*---------------------------------*
 * File:	xdefaults.c
 *----------------------------------------------------------------------*
 * $Id: xdefaults.c,v 1.108 2003/02/28 00:58:43 gcw Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *				- original version
 * Copyright (c) 1997,1998 mj olesen <olesen@me.queensu.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 * get resources from ~/.Xdefaults or ~/.Xresources with the memory-saving
 * default or with XGetDefault() (#define USE_XGETDEFAULT)
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "version.h"
#include "xdefaults.intpro"	/* PROTOS for internal routines */

/* #define DEBUG_RESOURCES */

static const char *const xnames[2] = { ".Xdefaults", ".Xresources" };

/*{{{ monolithic option/resource structure: */
/*
 * `string' options MUST have a usage argument
 * `switch' and `boolean' options have no argument
 * if there's no desc(ription), it won't appear in rxvt_usage()
 */

/* INFO() - descriptive information only */
#define INFO(opt, arg, desc)					\
    {0, -1, NULL, (opt), (arg), (desc)}

/* STRG() - command-line option, with/without resource */
#define STRG(rsp, kw, opt, arg, desc)				\
    {0, (rsp), (kw), (opt), (arg), (desc)}

/* RSTRG() - resource/long-option */
#define RSTRG(rsp, kw, arg)					\
    {0, (rsp), (kw), NULL, (arg), NULL}

/* BOOL() - regular boolean `-/+' flag */
#define BOOL(rsp, kw, opt, flag, desc)				\
    {(Opt_Boolean|(flag)), (rsp), (kw), (opt), NULL, (desc)}

/* SWCH() - `-' flag */
#define SWCH(opt, flag, desc)					\
    {(flag), -1, NULL, (opt), NULL, (desc)}

/* convenient macros */
#define optList_strlen(i)						\
    (optList[i].flag ? 0 : (optList[i].arg ? STRLEN(optList[i].arg) : 1))
#define optList_isBool(i)						\
    (optList[i].flag & Opt_Boolean)
#define optList_isReverse(i)						\
    (optList[i].flag & Opt_Reverse)
#define optList_size()							\
    (sizeof(optList) / sizeof(optList[0]))

static const struct {
    const unsigned long flag;	/* Option flag */
    const int       doff;	/* data offset */
    const char     *kw;		/* keyword */
    const char     *opt;	/* option */
    const char     *arg;	/* argument */
    const char     *desc;	/* description */
} optList[] = {
    STRG(Rs_display_name, NULL, "d", NULL, NULL),	/* short form */
    STRG(Rs_display_name, NULL, "display", "string", "X server to contact"),
    STRG(Rs_term_name, "termName", "tn", "string",
	 "value of the TERM environment variable"),
    STRG(Rs_geometry, NULL, "g", NULL, NULL),	/* short form */
    STRG(Rs_geometry, "geometry", "geometry", "geometry",
	 "size (in characters) and position"),
    SWCH("C", Opt_console, "intercept console messages"),
    SWCH("iconic", Opt_iconic, "start iconic"),
    SWCH("ic", Opt_iconic, NULL),	/* short form */
    BOOL(Rs_reverseVideo, "reverseVideo", "rv", Opt_reverseVideo,
	 "reverse video"),
    BOOL(Rs_loginShell, "loginShell", "ls", Opt_loginShell, "login shell"),
    BOOL(Rs_jumpScroll, "jumpScroll", "j", Opt_jumpScroll, "jump scrolling"),
#ifdef HAVE_SCROLLBARS
    BOOL(Rs_scrollBar, "scrollBar", "sb", Opt_scrollBar, "scrollbar"),
    BOOL(Rs_scrollBar_right, "scrollBar_right", "sr", Opt_scrollBar_right,
	 "scrollbar right"),
    BOOL(Rs_scrollBar_floating, "scrollBar_floating", "st",
	 Opt_scrollBar_floating, "scrollbar without a trough"),
#endif
    BOOL(Rs_scrollTtyOutput, "scrollTtyOutput", NULL, Opt_scrollTtyOutput,
	 NULL),
    BOOL(Rs_scrollTtyOutput, NULL, "si", Opt_Reverse | Opt_scrollTtyOutput,
	 "scroll-on-tty-output inhibit"),
    BOOL(Rs_scrollTtyKeypress, "scrollTtyKeypress", "sk", Opt_scrollTtyKeypress,
	 "scroll-on-keypress"),
    BOOL(Rs_scrollWithBuffer, "scrollWithBuffer", "sw", Opt_scrollWithBuffer,
	 "scroll-with-buffer"),
#ifdef TRANSPARENT
    BOOL(Rs_transparent, "inheritPixmap", "ip", Opt_transparent,
	 "inherit parent pixmap"),
    BOOL(Rs_transparent_all, "inheritPixmapforce", "ipf", Opt_transparent_all,
	 "forcefully inherit root pixmap"),
    SWCH("tr", Opt_transparent, NULL),
#endif
    BOOL(Rs_utmpInhibit, "utmpInhibit", "ut", Opt_utmpInhibit, "utmp inhibit"),
#ifndef NO_BELL
    BOOL(Rs_visualBell, "visualBell", "vb", Opt_visualBell, "visual bell"),
# if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
    BOOL(Rs_mapAlert, "mapAlert", NULL, Opt_mapAlert, NULL),
# endif
#endif
#ifdef META8_OPTION
    BOOL(Rs_meta8, "meta8", NULL, Opt_meta8, NULL),
#endif
#ifdef MOUSE_WHEEL
    BOOL(Rs_mouseWheelScrollPage, "mouseWheelScrollPage", NULL, Opt_mouseWheelScrollPage,
	 NULL),
#endif
#ifdef MULTICHAR_SET
    BOOL(Rs_mc_hack, "multibyte_cursor", "mcc", Opt_mc_hack,
	 "Multibyte character cursor movement"),
#endif
#ifndef NO_FRILLS
    BOOL(Rs_tripleclickwords, "tripleclickwords", "tcw", Opt_tripleclickwords,
	 "triple click word selection"),
#endif
    STRG(Rs_color + Color_bg, "background", "bg", "color", "background color"),
    STRG(Rs_color + Color_fg, "foreground", "fg", "color", "foreground color"),
    RSTRG(Rs_color + minCOLOR + 0, "color0", "color"),
    RSTRG(Rs_color + minCOLOR + 1, "color1", "color"),
    RSTRG(Rs_color + minCOLOR + 2, "color2", "color"),
    RSTRG(Rs_color + minCOLOR + 3, "color3", "color"),
    RSTRG(Rs_color + minCOLOR + 4, "color4", "color"),
    RSTRG(Rs_color + minCOLOR + 5, "color5", "color"),
    RSTRG(Rs_color + minCOLOR + 6, "color6", "color"),
    RSTRG(Rs_color + minCOLOR + 7, "color7", "color"),
#ifndef NO_BRIGHTCOLOR
    RSTRG(Rs_color + minBrightCOLOR + 0, "color8", "color"),
    RSTRG(Rs_color + minBrightCOLOR + 1, "color9", "color"),
    RSTRG(Rs_color + minBrightCOLOR + 2, "color10", "color"),
    RSTRG(Rs_color + minBrightCOLOR + 3, "color11", "color"),
    RSTRG(Rs_color + minBrightCOLOR + 4, "color12", "color"),
    RSTRG(Rs_color + minBrightCOLOR + 5, "color13", "color"),
    RSTRG(Rs_color + minBrightCOLOR + 6, "color14", "color"),
    RSTRG(Rs_color + minBrightCOLOR + 7, "color15", "color"),
#endif				/* NO_BRIGHTCOLOR */
#ifndef NO_BOLD_UNDERLINE_REVERSE
    RSTRG(Rs_color + Color_BD, "colorBD", "color"),
    RSTRG(Rs_color + Color_UL, "colorUL", "color"),
    RSTRG(Rs_color + Color_RV, "colorRV", "color"),
#endif				/* ! NO_BOLD_UNDERLINE_REVERSE */
#ifdef KEEP_SCROLLCOLOR
    RSTRG(Rs_color + Color_scroll, "scrollColor", "color"),
    RSTRG(Rs_color + Color_trough, "troughColor", "color"),
#endif				/* KEEP_SCROLLCOLOR */
#ifdef OPTION_HC
    STRG(Rs_color + Color_HC, "highlightColor", "hc", "color", "highlight color"),
#endif
#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
    RSTRG(Rs_path, "path", "search path"),
#endif				/* defined (XPM_BACKGROUND) || (MENUBAR_MAX) */
#ifdef XPM_BACKGROUND
    STRG(Rs_backgroundPixmap, "backgroundPixmap", "pixmap", "file[;geom]",
	 "background pixmap"),
#endif				/* XPM_BACKGROUND */
#if (MENUBAR_MAX)
    RSTRG(Rs_menu, "menu", "name[;tag]"),
#endif
#ifndef NO_BOLDFONT
    STRG(Rs_boldFont, "boldFont", "fb", "fontname", "bold text font"),
#endif
    STRG(Rs_font + 0, "font", "fn", "fontname", "normal text font"),
/* fonts: command-line option = resource name */
#if MAX_NFONTS > 1
    RSTRG(Rs_font + 1, "font1", "fontname"),
#endif
#if MAX_NFONTS > 2
    RSTRG(Rs_font + 2, "font2", "fontname"),
#endif
#if MAX_NFONTS > 3
    RSTRG(Rs_font + 3, "font3", "fontname"),
#endif
#if MAX_NFONTS > 4
    RSTRG(Rs_font + 4, "font4", "fontname"),
#endif
#if MAX_NFONTS > 5
    RSTRG(Rs_font + 5, "font5", "fontname"),
#endif
#if MAX_NFONTS > 6
    RSTRG(Rs_font + 6, "font6", "fontname"),
#endif
#if MAX_NFONTS > 7
    RSTRG(Rs_font + 7, "font7", "fontname"),
#endif
#ifdef MULTICHAR_SET
    STRG(Rs_mfont + 0, "mfont", "fm", "fontname", "multichar font"),
/* fonts: command-line option = resource name */
# if MAX_NFONTS > 1
    RSTRG(Rs_mfont + 1, "mfont1", "fontname"),
# endif
# if MAX_NFONTS > 2
    RSTRG(Rs_mfont + 2, "mfont2", "fontname"),
# endif
# if MAX_NFONTS > 3
    RSTRG(Rs_mfont + 3, "mfont3", "fontname"),
# endif
# if MAX_NFONTS > 4
    RSTRG(Rs_mfont + 4, "mfont4", "fontname"),
# endif
# if MAX_NFONTS > 5
    RSTRG(Rs_mfont + 5, "mfont5", "fontname"),
# endif
# if MAX_NFONTS > 6
    RSTRG(Rs_mfont + 6, "mfont6", "fontname"),
# endif
# if MAX_NFONTS > 7
    RSTRG(Rs_mfont + 7, "mfont7", "fontname"),
# endif
#endif				/* MULTICHAR_SET */
#ifdef MULTICHAR_SET
    STRG(Rs_multichar_encoding, "multichar_encoding", "km", "mode",
	 "multichar encoding; mode = eucj|sjis|big5|gb|kr|noenc"),
#endif				/* MULTICHAR_SET */
#ifdef USE_XIM
    STRG(Rs_inputMethod, "inputMethod", "im", "name", "name of input method"),
    STRG(Rs_preeditType, "preeditType", "pt", "style",
	 "input style: style = OverTheSpot|OffTheSpot|Root"),
#endif				/* USE_XIM */
#ifdef GREEK_SUPPORT
    STRG(Rs_greek_keyboard, "greek_keyboard", "grk", "mode",
	 "greek keyboard mapping; mode = iso | ibm"),
    RSTRG(Rs_greektoggle_key, "greektoggle_key", "keysym"),
#endif
    STRG(Rs_name, NULL, "name", "string",
	 "client instance, icon, and title strings"),
    STRG(Rs_title, "title", "title", "string", "title name for window"),
    STRG(Rs_title, NULL, "T", NULL, NULL),	/* short form */
    STRG(Rs_iconName, "iconName", "n", "string", "icon name for window"),
#ifndef NO_CURSORCOLOR
    STRG(Rs_color + Color_cursor, "cursorColor", "cr", "color", "cursor color"),
/* command-line option = resource name */
    RSTRG(Rs_color + Color_cursor2, "cursorColor2", "color"),
#endif				/* NO_CURSORCOLOR */
    STRG(Rs_color + Color_pointer, "pointerColor", "pr", "color",
	 "pointer color"),
    STRG(Rs_color + Color_border, "borderColor", "bd", "color",
	 "border color"),
    STRG(Rs_saveLines, "saveLines", "sl", "number",
	 "number of scrolled lines to save"),
#ifndef NO_FRILLS
    STRG(Rs_ext_bwidth, "externalBorder", "w", "number",
	 "external border in pixels"),
    STRG(Rs_ext_bwidth, NULL, "bw", NULL, NULL),
    STRG(Rs_ext_bwidth, NULL, "borderwidth", NULL, NULL),
    STRG(Rs_int_bwidth, "internalBorder", "b", "number",
	 "internal border in pixels"),
#endif
#ifndef NO_LINESPACE
    STRG(Rs_lineSpace, "lineSpace", "lsp", "number",
	 "number of extra pixels between rows"),
#endif
    STRG(Rs_scrollBar_thickness, "thickness", "sbt", "number",
	 "scrollbar thickness/width in pixels"),
#ifndef NO_BACKSPACE_KEY
    RSTRG(Rs_backspace_key, "backspacekey", "string"),
#endif
#ifndef NO_DELETE_KEY
    RSTRG(Rs_delete_key, "deletekey", "string"),
#endif
    RSTRG(Rs_selectstyle, "selectstyle", "mode"),
    RSTRG(Rs_scrollstyle, "scrollstyle", "mode"),
#ifdef HAVE_SCROLLBARS
    RSTRG(Rs_scrollBar_align, "scrollBar_align", "mode"),
#endif
#ifdef PRINTPIPE
    RSTRG(Rs_print_pipe, "print-pipe", "string"),
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
    RSTRG(Rs_bigfont_key, "bigfont_key", "keysym"),
    RSTRG(Rs_smallfont_key, "smallfont_key", "keysym"),
#endif
    STRG(Rs_modifier, "modifier", "mod", "modifier",
	 "meta modifier = alt|meta|hyper|super|mod1|...|mod5"),
    INFO("xrm", "string", "X resource"),
#ifdef CUTCHAR_RESOURCE
    RSTRG(Rs_cutchars, "cutchars", "string"),
#endif				/* CUTCHAR_RESOURCE */
    RSTRG(Rs_answerbackstring, "answerbackString", "string"),
    INFO("e", "command arg ...", "command to execute")
};

#undef INFO
#undef STRG
#undef RSTRG
#undef SWCH
#undef BOOL
/*}}} */

static const char releasestring[] = "Rxvt v" VERSION " - released: " DATE "\n";
static const char optionsstring[] = "Options: "
#if defined(XPM_BACKGROUND)
    "XPM,"
#endif
#if defined(TRANSPARENT)
    "transparent,"
#endif
#if defined(UTMP_SUPPORT)
    "utmp,"
#endif
#if defined(MENUBAR)
    "menubar,"
#endif
#if !defined(NO_FRILLS)
    "frills,"
#endif
#if !defined(NO_LINESPACE)
    "linespace,"
#endif
#if defined(PREFER_24BIT)
    "24bit,"
#endif
#if defined(USE_XIM)
    "XIM,"
#endif
#if defined(MULTICHAR_SET)
    "multichar_languages,"
#endif
    "scrollbars="
#if !defined(HAVE_SCROLLBARS)
    "NONE"
#else
# if defined(RXVT_SCROLLBAR)
    "rxvt"
#  if defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR)
    "+"
#  endif
# endif
# if defined(NEXT_SCROLLBAR)
    "NeXT"
#  if defined(XTERM_SCROLLBAR)
    "+"
#  endif
# endif
# if defined(XTERM_SCROLLBAR)
    "xterm"
# endif
#endif
    ","
#if defined(GREEK_SUPPORT)
    "Greek,"
#endif
#if defined(RXVT_GRAPHICS)
    "graphics,"
#endif
#if defined(NO_BACKSPACE_KEY)
    "no_backspace,"
#endif
#if defined(NO_DELETE_KEY)
    "no_delete,"
#endif
#if !defined(NO_STRINGS)
    "strings,"
#endif
#if defined(TTY_256COLOR)
    "256colour,"
#endif
#if defined(NO_RESOURCES)
    "NoResources"
#else
# if defined(USE_XGETDEFAULT)
    "XGetDefaults"
# else
    ".Xdefaults"
# endif
#endif
    "\nUsage: ";		/* Usage */

#define INDENT 18

/*{{{ usage: */
/*----------------------------------------------------------------------*/
/* EXTPROTO */
void
rxvt_usage(int type)
{
    unsigned int    i, col;

    write(STDERR_FILENO, releasestring, sizeof(releasestring) - 1);
    write(STDERR_FILENO, optionsstring, sizeof(optionsstring) - 1);
    write(STDERR_FILENO, APL_NAME, sizeof(APL_NAME) - 1);

    switch (type) {
    case 0:			/* brief listing */
	fprintf(stderr, " [-help] [--help]\n");
	for (col = 1, i = 0; i < optList_size(); i++)
	    if (optList[i].desc != NULL) {
		int             len = 0;

		if (!optList_isBool(i)) {
		    len = optList_strlen(i);
		    if (len > 0)
			len++;	/* account for space */
		}
#ifdef DEBUG_STRICT
		assert(optList[i].opt != NULL);
#endif
		len += 4 + STRLEN(optList[i].opt) + (optList_isBool(i) ? 2: 0);
		col += len;
		if (col > 79) {	/* assume regular width */
		    putc('\n', stderr);
		    col = 1 + len;
		}
		fprintf(stderr, " [-%s%s", (optList_isBool(i) ? "/+" : ""),
			optList[i].opt);
		if (optList_strlen(i))
		    fprintf(stderr, " %s]", optList[i].arg);
		else
		    fprintf(stderr, "]");
	    }
	break;

    case 1:			/* full command-line listing */
	fprintf(stderr, " [options] [-e command args]\n\n"
		"where options include:\n");
	for (i = 0; i < optList_size(); i++)
	    if (optList[i].desc != NULL) {
#ifdef DEBUG_STRICT
		assert(optList[i].opt != NULL);
#endif
		fprintf(stderr, "  %s%s %-*s%s%s\n",
			(optList_isBool(i) ? "-/+" : "-"), optList[i].opt,
			(INDENT - STRLEN(optList[i].opt)
			 + (optList_isBool(i) ? 0 : 2)),
			(optList[i].arg ? optList[i].arg : ""),
			(optList_isBool(i) ? "turn on/off " : ""),
			optList[i].desc);
	    }
	fprintf(stderr, "\n  --help to list long-options");
	break;

    case 2:			/* full resource listing */
	fprintf(stderr,
		" [options] [-e command args]\n\n"
		"where resources (long-options) include:\n");

	for (i = 0; i < optList_size(); i++)
	    if (optList[i].kw != NULL)
		fprintf(stderr, "  %s: %*s%s\n",
			optList[i].kw,
			(INDENT - STRLEN(optList[i].kw)), "", /* XXX */
			(optList_isBool(i) ? "boolean" : optList[i].arg));
#ifdef KEYSYM_RESOURCE
	fprintf(stderr, "  " "keysym.sym" ": %*s%s\n",
		(INDENT - sizeof("keysym.sym") + 1), "", /* XXX */
		"keysym");
#endif
	fprintf(stderr, "\n  -help to list options");
	break;
    }
    fprintf(stderr, "\n\n");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
}

/*}}} */

/*{{{ get command-line options before getting resources */
/* EXTPROTO */
void
rxvt_get_options(rxvt_t *r, int argc, const char *const *argv)
{
    int             i, bad_option = 0;
    static const char On[3] = "ON", Off[4] = "OFF";

    for (i = 1; i < argc; i++) {
	unsigned int    entry, longopt = 0;
	const char     *flag, *opt;

	opt = argv[i];
#ifdef DEBUG_RESOURCES
	fprintf(stderr, "argv[%d] = %s: ", i, opt);
#endif
	if (*opt == '-') {
	    flag = On;
	    if (*++opt == '-')
		longopt = *opt++;	/* long option */
	} else if (*opt == '+') {
	    flag = Off;
	    if (*++opt == '+')
		longopt = *opt++;	/* long option */
	} else {
	    bad_option = 1;
	    rxvt_print_error("bad option \"%s\"", opt);
	    continue;
	}

	if (!STRCMP(opt, "help"))
	    rxvt_usage(longopt ? 2 : 1);
	if (!STRCMP(opt, "h"))
	    rxvt_usage(0);

	/* feature: always try to match long-options */
	for (entry = 0; entry < optList_size(); entry++)
	    if ((optList[entry].kw && !STRCMP(opt, optList[entry].kw))
		|| (!longopt
		    && optList[entry].opt && !STRCMP(opt, optList[entry].opt)))
		break;

	if (entry < optList_size()) {
	    if (optList_isReverse(entry))
		flag = flag == On ? Off : On;
	    if (optList_strlen(entry)) {	/* string value */
		const char     *str = argv[++i];

#ifdef DEBUG_RESOURCES
		fprintf(stderr, "string (%s,%s) = ",
			optList[entry].opt ? optList[entry].opt : "nil",
			optList[entry].kw ? optList[entry].kw : "nil");
#endif
		if (flag == On && str && (optList[entry].doff != -1)) {
#ifdef DEBUG_RESOURCES
		    fprintf(stderr, "\"%s\"\n", str);
#endif
		    r->h->rs[optList[entry].doff] = str;
		    /*
		     * special cases are handled in main.c:main() to allow
		     * X resources to set these values before we settle for
		     * default values
		     */
		}
#ifdef DEBUG_RESOURCES
		else
		    fprintf(stderr, "???\n");
#endif
	    } else {		/* boolean value */
#ifdef DEBUG_RESOURCES
		fprintf(stderr, "boolean (%s,%s) = %s\n",
			optList[entry].opt, optList[entry].kw, flag);
#endif
		if (flag == On)
		    r->Options |= (optList[entry].flag);
		else
		    r->Options &= ~(optList[entry].flag);

		if (optList[entry].doff != -1)
		    r->h->rs[optList[entry].doff] = flag;
	    }
	} else
#ifdef KEYSYM_RESOURCE
	    /* if (!STRNCMP(opt, "keysym.", sizeof("keysym.") - 1)) */
	if (rxvt_Str_match(opt, "keysym.")) {
	    const char     *str = argv[++i];

	    if (str != NULL)
		rxvt_parse_keysym(r, opt + sizeof("keysym.") - 1, str);
	} else
#endif
	{
	    /*
	     * various old-style options, just ignore
	     * Obsolete since about Jan 96,
	     * so they can probably eventually be removed
	     */
	    const char     *msg = "bad";

	    if (longopt) {
		opt--;
		bad_option = 1;
	    } else if (!STRCMP(opt, "7") || !STRCMP(opt, "8")
#ifdef GREEK_SUPPORT
		       /* obsolete 12 May 1996 (v2.17) */
		       || !rxvt_Str_match(opt, "grk")
#endif
		)
		msg = "obsolete";
	    else
		bad_option = 1;

	    rxvt_print_error("%s option \"%s\"", msg, --opt);
	}
    }

    if (bad_option)
	rxvt_usage(0);
}

/*}}} */

#ifndef NO_RESOURCES
/*----------------------------------------------------------------------*/

# ifdef KEYSYM_RESOURCE
/*
 * Define key from XrmEnumerateDatabase.
 *   quarks will be something like
 *      "rxvt" "keysym" "0xFF01"
 *   value will be a string
 */
/* ARGSUSED */
/* INTPROTO */
Bool
rxvt_define_key(XrmDatabase *database __attribute__((unused)), XrmBindingList bindings __attribute__((unused)), XrmQuarkList quarks, XrmRepresentation *type __attribute__((unused)), XrmValue *value, XPointer closure __attribute__((unused)))
{
    int             last;
    rxvt_t         *r = rxvt_get_r();

    for (last = 0; quarks[last] != NULLQUARK; last++)	/* look for last quark in list */
	;
    last--;
    rxvt_parse_keysym(r, XrmQuarkToString(quarks[last]), (char *)value->addr);
    return False;
}

/*
 * look for something like this (XK_Delete)
 * rxvt*keysym.0xFFFF: "\177"
 *
 * arg will be
 *      NULL for ~/.Xdefaults and
 *      non-NULL for command-line options (need to allocate)
 */
#define NEWARGLIM	500	/* `reasonable' size */
/* INTPROTO */
int
rxvt_parse_keysym(rxvt_t *r, const char *str, const char *arg)
{
    int             n, sym;
    char           *key_string, *newarg = NULL;
    char            newargstr[NEWARGLIM];

    if (arg == NULL) {
	if ((n = rxvt_Str_match(str, "keysym.")) == 0)
	    return 0;
	str += n;		/* skip `keysym.' */
    }
/* some scanf() have trouble with a 0x prefix */
    if (isdigit(str[0])) {
	if (str[0] == '0' && toupper(str[1]) == 'X')
	    str += 2;
	if (arg) {
	    if (sscanf(str, (STRCHR(str, ':') ? "%x:" : "%x"), &sym) != 1)
		return -1;
	} else {
	    if (sscanf(str, "%x:", &sym) != 1)
		return -1;

	    /* cue to ':', it's there since sscanf() worked */
	    STRNCPY(newargstr, STRCHR(str, ':') + 1, NEWARGLIM - 1);
	    newargstr[NEWARGLIM - 1] = '\0';
	    newarg = newargstr;
	}
    } else {
	/*
	 * convert keysym name to keysym number
	 */
	STRNCPY(newargstr, str, NEWARGLIM - 1);
	newargstr[NEWARGLIM - 1] = '\0';
	if (arg == NULL) {
	    if ((newarg = STRCHR(newargstr, ':')) == NULL)
		return -1;
	    *newarg++ = '\0';	/* terminate keysym name */
	}
	if ((sym = XStringToKeysym(newargstr)) == None)
	    return -1;
    }

    if (sym < 0xFF00 || sym > 0xFFFF)	/* we only do extended keys */
	return -1;
    sym &= 0xFF;
    if (r->h->Keysym_map[sym] != NULL)	/* already set ? */
	return -1;

    if (newarg == NULL) {
	STRNCPY(newargstr, arg, NEWARGLIM - 1);
	newargstr[NEWARGLIM - 1] = '\0';
	newarg = newargstr;
    }
    rxvt_Str_trim(newarg);
    if (*newarg == '\0' || (n = rxvt_Str_escaped(newarg)) == 0)
	return -1;
    MIN_IT(n, 255);
    key_string = rxvt_malloc((n + 1) * sizeof(char));

    key_string[0] = n;
    STRNCPY(key_string + 1, newarg, n);
    r->h->Keysym_map[sym] = (unsigned char *)key_string;

    return 1;
}

# endif				/* KEYSYM_RESOURCE */

# ifndef USE_XGETDEFAULT
/*{{{ rxvt_get_xdefaults() */
/*
 * the matching algorithm used for memory-save fake resources
 */
/* INTPROTO */
void
rxvt_get_xdefaults(rxvt_t *r, FILE *stream, const char *name)
{
    unsigned int    len;
    char           *str, buffer[256];

    if (stream == NULL)
	return;
    len = STRLEN(name);
    while ((str = fgets(buffer, sizeof(buffer), stream)) != NULL) {
	unsigned int    entry, n;

	while (*str && isspace(*str))
	    str++;		/* leading whitespace */

	if ((str[len] != '*' && str[len] != '.')
	    || (len && STRNCMP(str, name, len)))
	    continue;
	str += (len + 1);	/* skip `name*' or `name.' */

# ifdef KEYSYM_RESOURCE
	if (!rxvt_parse_keysym(r, str, NULL))
# endif				/* KEYSYM_RESOURCE */
	    for (entry = 0; entry < optList_size(); entry++) {
		const char     *kw = optList[entry].kw;

		if (kw == NULL)
		    continue;
		n = STRLEN(kw);
		if (str[n] == ':' && rxvt_Str_match(str, kw)) {
		    /* skip `keyword:' */
		    str += (n + 1);
		    rxvt_Str_trim(str);
		    n = STRLEN(str);
		    if (n && r->h->rs[optList[entry].doff] == NULL) {
			/* not already set */
			int             s;
			char           *p = rxvt_malloc((n + 1) * sizeof(char));

			STRCPY(p, str);
			r->h->rs[optList[entry].doff] = p;
			if (optList_isBool(entry)) {
			    s = STRCASECMP(str, "TRUE") == 0
				|| STRCASECMP(str, "YES") == 0
				|| STRCASECMP(str, "ON") == 0
				|| STRCASECMP(str, "1") == 0;
			    if (optList_isReverse(entry))
				s = !s;
			    if (s)
				r->Options |= (optList[entry].flag);
			    else
				r->Options &= ~(optList[entry].flag);
			}
		    }
		    break;
		}
	    }
    }
    rewind(stream);
}

/*}}} */
# endif				/* ! USE_XGETDEFAULT */
#endif				/* NO_RESOURCES */

/*{{{ read the resources files */
/*
 * using XGetDefault() or the hand-rolled replacement
 */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_extract_resources(rxvt_t *r, Display *display __attribute__((unused)), const char *name)
{
#ifndef NO_RESOURCES

# if defined XAPPLOADDIR
#  if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
    /* Compute the path of the possibly available localized Rxvt file */ 
    char           *localepath = NULL;

    if (r->h->locale != NULL) {	/* XXX: must limit length of string */
	localepath = rxvt_malloc(256); 
	sprintf(localepath, XAPPLOADDIRLOCALE "/" APL_SUBCLASS,
		(int)(258 - sizeof(XAPPLOADDIRLOCALE) - sizeof(APL_SUBCLASS)),
		r->h->locale);	/* 258 = 255 + 4 (-.*s) - 1 (/) */
    }

    {
#  endif
# endif

# ifdef USE_XGETDEFAULT
/*
 * get resources using the X library function
 */
    int             entry;

#  ifdef XrmEnumOneLevel
    int             i;
    char           *displayResource, *xe;
    XrmName         name_prefix[3];
    XrmClass        class_prefix[3];
    XrmDatabase     database, rdb1;
    char            fname[1024];

    XrmInitialize();
    database = NULL;

/* Get any Xserver defaults */

    displayResource = XResourceManagerString(display);
    if (displayResource != NULL)
	database = XrmGetStringDatabase(displayResource);

#   ifdef HAVE_EXTRA_XRESOURCE_FILES
/* Add in ~/.Xdefaults or ~/.Xresources */
    {
	char           *ptr;

	if ((ptr = (char *)getenv("HOME")) == NULL)
	    ptr = ".";

	for (i = 0; i < (sizeof(xnames) / sizeof(xnames[0])); i++) {
	    sprintf(fname, "%-.*s/%s", sizeof(fname) - STRLEN(xnames[i]) - 2,
		    ptr, xnames[i]);
	    if ((rdb1 = XrmGetFileDatabase(fname)) != NULL) {
		XrmMergeDatabases(rdb1, &database);
#    ifndef HAVE_BOTH_XRESOURCE_FILES
		break;
#    endif
	    }
	}
    }
#   endif

/* Add in XENVIRONMENT file */

    if ((xe = (char *)getenv("XENVIRONMENT")) != NULL
	&& (rdb1 = XrmGetFileDatabase(xe)) != NULL)
	XrmMergeDatabases(rdb1, &database);

/* Add in Rxvt file */
#   if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
    if (localepath == NULL || (rdb1 = XrmGetFileDatabase(localepath)) == NULL)
#   endif
    rdb1 = XrmGetFileDatabase(XAPPLOADDIR "/" APL_SUBCLASS);
    if (rdb1 != NULL)
        XrmMergeDatabases(rdb1, &database);

/* Add in $XAPPLRESDIR/Rxvt only; not bothering with XUSERFILESEARCHPATH */
    if ((xe = (char *)getenv("XAPPLRESDIR")) != NULL) {
	sprintf(fname, "%-.*s/" APL_SUBCLASS, sizeof(fname)
		- sizeof(APL_SUBCLASS) - 2, xe);
	if ((rdb1 = XrmGetFileDatabase(fname)) != NULL)
	    XrmMergeDatabases(rdb1, &database);
    }

    XrmSetDatabase(display, database);
#  endif

/*
 * Query resources for options that affect us
 */
    for (entry = 0; entry < optList_size(); entry++) {
	int             s;
	char           *p, *p0;
	const char     *kw = optList[entry].kw;

	if (kw == NULL || r->h->rs[optList[entry].doff] != NULL)
	    continue;		/* previously set */

	p = XGetDefault(display, name, kw);
	p0 = XGetDefault(display, "!INVALIDPROGRAMMENAMEDONTMATCH!", kw);
	if (p == NULL || (p0 && STRCMP(p, p0) == 0)) {
	    p = XGetDefault(display, APL_SUBCLASS, kw);
	    if (p == NULL || (p0 && STRCMP(p, p0) == 0))
		p = XGetDefault(display, APL_CLASS, kw);
	}
	if (p == NULL && p0)
	    p = p0;
	if (p) {
	    r->h->rs[optList[entry].doff] = p;

	    if (optList_isBool(entry)) {
		s = STRCASECMP(p, "TRUE") == 0
		    || STRCASECMP(p, "YES") == 0
		    || STRCASECMP(p, "ON") == 0
		    || STRCASECMP(p, "1") == 0;
		if (optList_isReverse(entry))
		    s = !s;
		if (s)
		    r->Options |= (optList[entry].flag);
		else
		    r->Options &= ~(optList[entry].flag);
	    }
	}
    }

/*
 * [R5 or later]: enumerate the resource database
 */
#  ifdef XrmEnumOneLevel
#   ifdef KEYSYM_RESOURCE
    name_prefix[0] = XrmStringToName(name);
    name_prefix[1] = XrmStringToName("keysym");
    name_prefix[2] = NULLQUARK;
    class_prefix[0] = XrmStringToName(APL_SUBCLASS);
    class_prefix[1] = XrmStringToName("Keysym");
    class_prefix[2] = NULLQUARK;
/* XXX: Need to check sizeof(rxvt_t) == sizeof(XPointer) */
    XrmEnumerateDatabase(XrmGetDatabase(display), name_prefix, class_prefix,
			 XrmEnumOneLevel, rxvt_define_key, NULL);
    name_prefix[0] = XrmStringToName(APL_CLASS);
    name_prefix[1] = XrmStringToName("keysym");
    class_prefix[0] = XrmStringToName(APL_CLASS);
    class_prefix[1] = XrmStringToName("Keysym");
/* XXX: Need to check sizeof(rxvt_t) == sizeof(XPointer) */
    XrmEnumerateDatabase(XrmGetDatabase(display), name_prefix, class_prefix,
			 XrmEnumOneLevel, rxvt_define_key, NULL);
#   endif
#  endif

# else				/* USE_XGETDEFAULT */
/* get resources the hard way, but save lots of memory */
    FILE           *fd = NULL;
    char           *home;

    if ((home = getenv("HOME")) != NULL) {
	unsigned int    i, len = STRLEN(home) + 2;
	char           *f = NULL;

	for (i = 0; i < (sizeof(xnames) / sizeof(xnames[0])); i++) {
	    f = rxvt_realloc(f, (len + STRLEN(xnames[i])) * sizeof(char));

	    sprintf(f, "%s/%s", home, xnames[i]);

	    if ((fd = fopen(f, "r")) != NULL)
		break;
	}
	free(f);
    }
/*
 * The normal order to match resources is the following:
 * @ global resources (partial match, ~/.Xdefaults)
 * @ application file resources (XAPPLOADDIR/Rxvt)
 * @ class resources (~/.Xdefaults)
 * @ private resources (~/.Xdefaults)
 *
 * However, for the hand-rolled resources, the matching algorithm
 * checks if a resource string value has already been allocated
 * and won't overwrite it with (in this case) a less specific
 * resource value.
 *
 * This avoids multiple allocation.  Also, when we've called this
 * routine command-line string options have already been applied so we
 * needn't to allocate for those resources.
 *
 * So, search in resources from most to least specific.
 *
 * Also, use a special sub-class so that we can use either or both of
 * "XTerm" and "Rxvt" as class names.
 */

    rxvt_get_xdefaults(r, fd, name);
    rxvt_get_xdefaults(r, fd, APL_SUBCLASS);

#  if defined(XAPPLOADDIR) && defined(USE_XAPPLOADDIR)
    {
	FILE           *ad = NULL;

#   if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
	if (localepath == NULL || (ad = fopen(localepath, "r")) == NULL)
#   endif
	ad = fopen(XAPPLOADDIR "/" APL_SUBCLASS, "r");
	if (ad != NULL) {
	    rxvt_get_xdefaults(r, ad, APL_SUBCLASS);
	    rxvt_get_xdefaults(r, ad, "");
	    fclose(ad);
	}
    }
#  endif			/* XAPPLOADDIR */

    rxvt_get_xdefaults(r, fd, APL_CLASS);
    rxvt_get_xdefaults(r, fd, "");	/* partial match */
    if (fd != NULL)
	fclose(fd);
# endif				/* USE_XGETDEFAULT */

# if defined XAPPLOADDIR
#  if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
    }

    /* Free the path of the possibly available localized Rxvt file */ 
    free(localepath);
#  endif
# endif

#endif				/* NO_RESOURCES */

/*
 * even without resources, at least do this setup for command-line
 * options and command-line long options
 */
#ifdef MULTICHAR_SET
    rxvt_set_multichar_encoding(r, r->h->rs[Rs_multichar_encoding]);
#endif
#ifdef GREEK_SUPPORT
/* this could be a function in grkelot.c */
/* void set_greek_keyboard (const char * str); */
    if (r->h->rs[Rs_greek_keyboard]) {
	if (!STRCMP(r->h->rs[Rs_greek_keyboard], "iso"))
	    greek_setmode(GREEK_ELOT928);	/* former -grk9 */
	else if (!STRCMP(r->h->rs[Rs_greek_keyboard], "ibm"))
	    greek_setmode(GREEK_IBM437);	/* former -grk4 */
    }
    {
	KeySym          sym;

	if (r->h->rs[Rs_greektoggle_key]
	    && ((sym = XStringToKeysym(r->h->rs[Rs_greektoggle_key])) != 0))
	    r->h->ks_greekmodeswith = sym;
    }
#endif				/* GREEK_SUPPORT */

#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
    {
	KeySym          sym;

	if (r->h->rs[Rs_bigfont_key]
	    && ((sym = XStringToKeysym(r->h->rs[Rs_bigfont_key])) != 0))
	    r->h->ks_bigfont = sym;
	if (r->h->rs[Rs_smallfont_key]
	    && ((sym = XStringToKeysym(r->h->rs[Rs_smallfont_key])) != 0))
	    r->h->ks_smallfont = sym;
    }
#endif
}

/*}}} */
/*----------------------- end-of-file (C source) -----------------------*/
