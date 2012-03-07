/*
 * Copyright 1995, 1996, 1999, 2000, 2001, 2002, 2004 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

/*
 *	resources.h
 *		x3270/c3270/s3270/tcl3270 resource and option names.
 */

/* Resources. */
#define ResActiveIcon		"activeIcon"
#define ResAdVersion		"adVersion"
#define ResAidWait		"aidWait"
#define ResAllBold		"allBold"
#define ResAllowResize		"allowResize"
#define ResAltCursor		"altCursor"
#define ResAltScreen		"altScreen"
#define ResAplMode		"aplMode"
#define ResAssocCommand		"printer.assocCommandLine"
#define ResBaselevelTranslations	"baselevelTranslations"
#define ResBellVolume		"bellVolume"
#define ResBlankFill		"blankFill"
#define ResBoldColor		"boldColor"
#define ResBsdTm		"bsdTm"
#define ResCbreak		"cbreak"
#define ResCertFile		"certFile"
#define ResCharClass		"charClass"
#define ResCharset		"charset"
#define ResCharsetList		"charsetList"
#define ResCodepage		"codepage"
#define ResColor8		"color8"
#define ResColorBackground	"colorBackground"
#define ResColorScheme		"colorScheme"
#define ResComposeMap		"composeMap"
#define ResConfDir		"confDir"
#define ResConnectFileName	"connectFileName"
#define ResCrosshair		"crosshair"
#define ResCursesKeypad		"cursesKeypad"
#define ResCursorBlink		"cursorBlink"
#define ResCursorColor		"cursorColor"
#define ResCursorPos		"cursorPos"
#define ResDbcsConverters	"dbcsConverters"
#define ResDebugFont		"debugFont"
#define ResDebugTracing		"debugTracing"
#define ResDefScreen		"defScreen"
#define ResDftBufferSize	"dftBufferSize"
#define ResDisconnectClear	"disconnectClear"
#define ResDisplayCharset	"displayCharset"
#define ResDoConfirms		"doConfirms"
#define ResDsTrace		"dsTrace"
#define ResEmulatorFont		"emulatorFont"
#define ResEof			"eof"
#define ResErase		"erase"
#define ResEventTrace		"eventTrace"
#define ResExtended		"extended"
#define ResFixedSize		"fixedSize"
#define ResFtCharset		"ftCharset"
#define ResFtCommand		"ftCommand"
#define ResHighlightBold	"highlightBold"
#define ResHostsFile		"hostsFile"
#define ResIconFont		"iconFont"
#define ResIconLabelFont	"iconLabelFont"
#define ResIcrnl		"icrnl"
#define ResIdleCommand		"idleCommand"
#define ResIdleCommandEnabled	"idleCommandEnabled"
#define ResIdleTimeout		"idleTimeout"
#define ResInlcr		"inlcr"
#define ResInputColor		"inputColor"
#define ResInputMethod		"inputMethod"
#define ResIntr			"intr"
#define ResInvertKeypadShift	"invertKeypadShift"
#define ResKeymap		"keymap"
#define ResKeypad		"keypad"
#define ResKeypadBackground	"keypadBackground"
#define ResKeypadOn		"keypadOn"
#define ResKill			"kill"
#define ResLabelIcon		"labelIcon"
#define ResLineWrap		"lineWrap"
#define ResLnext		"lnext"
#define ResLocalEncoding	"localEncoding"
#define ResLockedCursor		"lockedCursor"
#define ResLuCommandLine	"printer.luCommandLine"
#define ResM3279		"m3279"
#define ResMacros		"macros"
#define ResMarginedPaste	"marginedPaste"
#define ResMenuBar		"menuBar"
#define ResMetaEscape		"metaEscape"
#define ResModel		"model"
#define ResModifiedSel		"modifiedSel"
#define ResModifiedSelColor	"modifiedSelColor"
#define ResMono			"mono"
#define ResMonoCase		"monoCase"
#define ResNoOther		"noOther"
#define ResNormalColor		"normalColor"
#define ResNormalCursor		"normalCursor"
#define ResNumericLock		"numericLock"
#define ResOerrLock		"oerrLock"
#define ResOnce			"once"
#define ResOnlcr		"onlcr"
#define ResOversize		"oversize"
#define ResPort			"port"
#define ResPreeditType		"preeditType"
#define ResPrinterCommand	"printer.command"
#define ResPrinterLu		"printerLu"
#define ResQuit			"quit"
#define ResReconnect		"reconnect"
#define ResRectangleSelect	"rectangleSelect"
#define ResRprnt		"rprnt"
#define ResSaveLines		"saveLines"
#define ResSchemeList		"schemeList"
#define ResScreenTrace		"screenTrace"
#define ResScreenTraceFile	"screenTraceFile"
#define ResScripted		"scripted"
#define ResScrollBar		"scrollBar"
#define ResSecure		"secure"
#define ResSelectBackground	"selectBackground"
#define ResShowTiming		"showTiming"
#define ResSuppressActions	"suppressActions"
#define ResSuppressHost		"suppressHost"
#define ResSuppressFontMenu	"suppressFontMenu"
#define ResSuppress		"suppress"
#define ResTermName		"termName"
#define ResTraceDir		"traceDir"
#define ResTraceFile		"traceFile"
#define ResTraceFileSize	"traceFileSize"
#define ResTraceMonitor		"traceMonitor"
#define ResTypeahead		"typeahead"
#define ResUnlockDelay		"unlockDelay"
#define ResUseCursorColor	"useCursorColor"
#define ResVisibleControl	"visibleControl"
#define ResVisualBell		"visualBell"
#define ResVisualSelect		"visualSelect"
#define ResVisualSelectColor	"visualSelectColor"
#define ResWaitCursor		"waitCursor"
#define ResWerase		"werase"

/* Dotted resource names. */
#define DotActiveIcon		"." ResActiveIcon
#define DotAplMode		"." ResAplMode
#define DotCertFile		"." ResCertFile
#define DotCbreak		"." ResCbreak
#define DotCharClass		"." ResCharClass
#define DotCharset		"." ResCharset
#define DotColorScheme		"." ResColorScheme
#define DotDsTrace		"." ResDsTrace
#define DotEmulatorFont		"." ResEmulatorFont
#define DotExtended		"." ResExtended
#define DotInputMethod		"." ResInputMethod
#define DotLocalEncoding	"." ResLocalEncoding
#define DotKeymap		"." ResKeymap
#define DotKeypadOn		"." ResKeypadOn
#define DotM3279		"." ResM3279
#define DotModel		"." ResModel
#define DotMono			"." ResMono
#define DotOnce			"." ResOnce
#define DotOversize		"." ResOversize
#define DotPort			"." ResPort
#define DotPreeditType		"." ResPreeditType
#define DotPrinterLu		"." ResPrinterLu
#define DotReconnect		"." ResReconnect
#define DotSaveLines		"." ResSaveLines
#define DotScripted		"." ResScripted
#define DotScrollBar		"." ResScrollBar
#define DotTermName		"." ResTermName
#define DotTraceFile		"." ResTraceFile
#define DotTraceFileSize	"." ResTraceFileSize

/* Resource classes. */
#define ClsActiveIcon		"ActiveIcon"
#define ClsAdVersion		"AdVersion"
#define ClsAidWait		"AidWait"
#define ClsAllBold		"AllBold"
#define ClsAllowResize		"AllowResize"
#define ClsAltCursor		"AltCursor"
#define ClsAplMode		"AplMode"
#define ClsBaselevelTranslations	"BaselevelTranslations"
#define ClsBellVolume		"BellVolume"
#define ClsBlankFill		"BlankFill"
#define ClsBoldColor		"BoldColor"
#define ClsBsdTm		"BsdTm"
#define ClsCbreak		"Cbreak"
#define ClsCertFile		"CertFile"
#define ClsCharClass		"CharClass"
#define ClsCharset		"Charset"
#define ClsColor8		"Color8"
#define ClsColorBackground	"ColorBackground"
#define ClsColorScheme		"ColorScheme"
#define ClsComposeMap		"ComposeMap"
#define ClsConfDir		"ConfDir"
#define ClsConnectFileName	"ConnectFileName"
#define ClsCrosshair		"Crosshair"
#define ClsCursorBlink		"CursorBlink"
#define ClsCursorColor		"CursorColor"
#define ClsCursorPos		"CursorPos"
#define ClsDebugFont		"DebugFont"
#define ClsDebugTracing		"DebugTracing"
#define ClsDftBufferSize	"DftBufferSize"
#define ClsDisconnectClear	"DisconnectClear"
#define ClsDoConfirms		"DoConfirms"
#define ClsDsTrace		"DsTrace"
#define ClsEmulatorFont		"EmulatorFont"
#define ClsEof			"Eof"
#define ClsErase		"Erase"
#define ClsEventTrace		"EventTrace"
#define ClsExtended		"Extended"
#define ClsFixedSize		"FixedSize"
#define ClsFtCommand		"FtCommand"
#define ClsHighlightBold	"HighlightBold"
#define ClsHostsFile		"HostsFile"
#define ClsIconFont		"IconFont"
#define ClsIconLabelFont	"IconLabelFont"
#define ClsIcrnl		"Icrnl"
#define ClsIdleCommand		"IdleCommand"
#define ClsIdleCommandEnabled	"IdleCommandEnabled"
#define ClsIdleTimeout		"IdleTimeout"
#define ClsInlcr		"Inlcr"
#define ClsInputColor		"InputColor"
#define ClsInputMethod		"InputMethod"
#define ClsIntr			"Intr"
#define ClsInvertKeypadShift	"InvertKeypadShift"
#define ClsKeymap		"Keymap"
#define ClsKeypad		"Keypad"
#define ClsKeypadBackground	"KeypadBackground"
#define ClsKeypadOn		"KeypadOn"
#define ClsKill			"Kill"
#define ClsLabelIcon		"LabelIcon"
#define ClsLineWrap		"LineWrap"
#define ClsLnext		"Lnext"
#define ClsLocalEncoding	"LocalEncoding"
#define ClsLockedCursor		"LockedCursor"
#define ClsM3279		"M3279"
#define ClsMacros		"Macros"
#define ClsMarginedPaste	"MarginedPaste"
#define ClsMenuBar		"MenuBar"
#define ClsMetaEscape		"MetaEscape"
#define ClsModel		"Model"
#define ClsModifiedSel		"ModifiedSel"
#define ClsModifiedSelColor	"ModifiedSelColor"
#define ClsMono			"Mono"
#define ClsMonoCase		"MonoCase"
#define ClsNoOther		"NoOther"
#define ClsNormalColor		"NormalColor"
#define ClsNormalCursor		"NormalCursor"
#define ClsNumericLock		"NumericLock"
#define ClsOerrLock		"OerrLock"
#define ClsOnce			"Once"
#define ClsOnlcr		"Onlcr"
#define ClsOversize		"Oversize"
#define ClsPort			"Port"
#define ClsPreeditType		"PreeditType"
#define ClsPrinterLu		"PrinterLu"
#define ClsQuit			"Quit"
#define ClsReconnect		"Reconnect"
#define ClsRectangleSelect	"RectangleSelect"
#define ClsRprnt		"Rprnt"
#define ClsSaveLines		"SaveLines"
#define ClsScreenTrace		"ScreenTrace"
#define ClsScreenTraceFile	"ScreenTraceFile"
#define ClsScripted		"Scripted"
#define ClsScrollBar		"ScrollBar"
#define ClsSecure		"Secure"
#define ClsSelectBackground	"SelectBackground"
#define ClsShowTiming		"ShowTiming"
#define ClsSuppressHost		"SuppressHost"
#define ClsSuppressFontMenu	"SuppressFontMenu"
#define ClsTermName		"TermName"
#define ClsTraceDir		"TraceDir"
#define ClsTraceFile		"TraceFile"
#define ClsTraceFileSize	"TraceFileSize"
#define ClsTraceMonitor		"TraceMonitor"
#define ClsTypeahead		"Typeahead"
#define ClsUnlockDelay		"UnlockDelay"
#define ClsUseCursorColor	"UseCursorColor"
#define ClsVisibleControl	"VisibleControl"
#define ClsVisualBell		"VisualBell"
#define ClsVisualSelect		"VisualSelect"
#define ClsVisualSelectColor	"VisualSelectColor"
#define ClsWaitCursor		"WaitCursor"
#define ClsWerase		"Werase"

/* Options. */
#define OptActiveIcon		"-activeicon"
#define OptAllBold		"-allbold"
#define OptAltScreen		"-altscreen"
#define OptAplMode		"-apl"
#define OptCbreak		"-cbreak"
#define OptCertFile		"-certificate"
#define OptCharClass		"-cc"
#define OptCharset		"-charset"
#define OptClear		"-clear"
#define OptColorScheme		"-scheme"
#define OptDefScreen		"-defscreen"
#define OptDsTrace		"-trace"
#define OptEmulatorFont		"-efont"
#define OptExtended		"-extended"
#define OptHostsFile		"-hostsfile"
#define OptIconName		"-iconname"
#define OptIconX		"-iconx"
#define OptIconY		"-icony"
#define OptInputMethod		"-im"
#define OptKeymap		"-keymap"
#define OptKeypadOn		"-keypad"
#define OptLocalEncoding	"-km"
#define OptLocalProcess		"-e"
#define OptM3279		"-color"
#define OptModel		"-model"
#define OptMono			"-mono"
#define OptNoScrollBar		"+sb"
#define OptOnce			"-once"
#define OptOversize		"-oversize"
#define OptPort			"-port"
#define OptPreeditType		"-pt"
#define OptPrinterLu		"-printerlu"
#define OptReconnect		"-reconnect"
#define OptSaveLines		"-sl"
#define OptSecure		"-secure"
#define OptScripted		"-script"
#define OptScrollBar		"-sb"
#define OptSet			"-set"
#define OptTermName		"-tn"
#define OptTraceFile		"-tracefile"
#define OptTraceFileSize	"-tracefilesize"

/* Miscellaneous values. */
#define ResTrue			"true"
#define ResFalse		"false"
#define KpLeft			"left"
#define KpRight			"right"
#define KpBottom		"bottom"
#define KpIntegral		"integral"
#define KpInsideRight		"insideRight"
#define Apl			"apl"

/* Resources that are gotten explicitly. */
#define ResComposeMap		"composeMap"
#define ResEmulatorFontList	"emulatorFontList"
#define ResKeyHeight		"keyHeight"
#define ResKeyWidth		"keyWidth"
#define ResLargeKeyWidth	"largeKeyWidth"
#define ResMessage		"message"
#define ResNvt			"nvt"
#define ResPaWidth		"paWidth"
#define ResPfWidth		"pfWidth"
#define ResPrintTextCommand	"printTextCommand"
#define ResPrintWindowCommand	"printWindowCommand"
#define ResTraceCommand		"traceCommand"
#define ResUser			"user"