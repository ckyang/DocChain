#############################################################################
# Makefile for building: DocChain.app/Contents/MacOS/DocChain
# Generated by qmake (3.1) (Qt 5.10.0)
# Project:  DocChain.pro
# Template: app
# Command: /Users/CK-Yang/Qt/5.10.0/ios/bin/qmake -o DocChain.xcodeproj/project.pbxproj DocChain.pro -spec macx-xcode
#############################################################################

MAKEFILE      = project.pbxproj

MOC       = /Users/CK-Yang/Qt/5.10.0/ios/bin/moc
UIC       = /Users/CK-Yang/Qt/5.10.0/ios/bin/uic
LEX       = flex
LEXFLAGS  = 
YACC      = yacc
YACCFLAGS = -d
DEFINES       = -DQT_COMPILER_SUPPORTS_SSE2 -DQT_DEPRECATED_WARNINGS -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB
INCPATH       = -I. -I../../Qt/5.10.0/ios/mkspecs/common/uikit -I../../Qt/5.10.0/ios/mkspecs/common/uikit -I. -I../../Qt/5.10.0/ios/include -I../../Qt/5.10.0/ios/include/QtWidgets -I../../Qt/5.10.0/ios/include/QtGui -I../../Qt/5.10.0/ios/include/QtCore -I. -I../../Qt/5.10.0/ios/mkspecs/macx-ios-clang
DEL_FILE  = rm -f
MOVE      = mv -f

preprocess: compilers
clean preprocess_clean: compiler_clean

mocclean: compiler_moc_header_clean compiler_moc_objc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_objc_header_make_all compiler_moc_source_make_all

check: first

benchmark: first

compilers: moc_dialog.cpp
compiler_rcc_make_all:
compiler_rcc_clean:
compiler_moc_header_make_all: moc_dialog.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_dialog.cpp
moc_dialog.cpp: ../../Qt/5.10.0/ios/include/QtWidgets/QDialog \
		../../Qt/5.10.0/ios/include/QtCore/QThread \
		gui/dialog.h \
		../../Qt/5.10.0/ios/bin/moc
	/Users/CK-Yang/Qt/5.10.0/ios/bin/moc $(DEFINES) -D__APPLE__ -D__GNUC__=4 -D__APPLE_CC__ -D__cplusplus=199711L -D__APPLE_CC__=6000 -D__clang__ -D__clang_major__=9 -D__clang_minor__=0 -D__clang_patchlevel__=0 -D__GNUC__=4 -D__GNUC_MINOR__=2 -D__GNUC_PATCHLEVEL__=1 -I/Users/CK-Yang/Qt/5.10.0/ios/mkspecs/macx-ios-clang -I/Users/CK-Yang/GitHub/DocChain -I/Users/CK-Yang/Qt/5.10.0/ios/mkspecs/common/uikit -I/Users/CK-Yang/Qt/5.10.0/ios/mkspecs/common/uikit -I/Users/CK-Yang/GitHub/DocChain -I/Users/CK-Yang/Qt/5.10.0/ios/include -I/Users/CK-Yang/Qt/5.10.0/ios/include/QtWidgets -I/Users/CK-Yang/Qt/5.10.0/ios/include/QtGui -I/Users/CK-Yang/Qt/5.10.0/ios/include/QtCore -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/9.0.0/include -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS11.2.sdk/usr/include gui/dialog.h -o moc_dialog.cpp

compiler_moc_objc_header_make_all:
compiler_moc_objc_header_clean:
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_rez_source_make_all:
compiler_rez_source_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean 
