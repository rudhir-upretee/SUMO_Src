/****************************************************************************/
/// @file    StringUtilsTests.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    2009
/// @version $Id: StringUtilsTest.cpp 11671 2012-01-07 20:14:30Z behrisch $
///
// Tests StringUtils class from <SUMO>/src/utils/common
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2010 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
// ===========================================================================
// included modules
// ===========================================================================
#include <gtest/gtest.h>
#include <utils/common/StringUtils.h>


// ===========================================================================
// test definitions
// ===========================================================================
/* Tests the method prune. Cut the blanks at the beginning and at the end of a string*/
TEST(StringUtils, test_method_prune) {
	EXPECT_EQ("result", StringUtils::prune("  result ")) << "Blanks at the beginning and at the end of a string must be removed.";
	EXPECT_EQ("", StringUtils::prune("  ")) << "Blanks at the beginning and at the end of a string must be removed.";
}

/* Tests the method to_lower_case.*/
TEST(StringUtils, test_method_to_lower_case) {
	EXPECT_EQ("hello", StringUtils::to_lower_case("HELLO"))<< "String should be converted into small letter.";
	EXPECT_EQ("world", StringUtils::to_lower_case("World"))<< "String should be converted into small letter.";
	std::string str;
	EXPECT_EQ("", StringUtils::to_lower_case(str));
}

/* Tests the method convertUmlaute.*/
TEST(StringUtils, test_method_convertUmlaute) {
	EXPECT_EQ("ae", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("Ae", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("oe", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("Oe", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("ue", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("Ue", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("ss", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("E", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("e", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("E", StringUtils::convertUmlaute("�"));
	EXPECT_EQ("e", StringUtils::convertUmlaute("�"));
}

/* Tests the method replace. */
TEST(StringUtils, test_method_replace) {	
	EXPECT_EQ("helt", StringUtils::replace("hello","lo","t"));
	EXPECT_EQ("heststo", StringUtils::replace("hello","l","st"));
	EXPECT_EQ("", StringUtils::replace("","l","st"));
}

/* Tests the method replace with empty string. */
TEST(StringUtils, test_method_replace_empty_string) {		
	EXPECT_EQ("", StringUtils::replace("","l","st"));
}

/* Tests the method replace with empty second_argument */
TEST(StringUtils, test_method_replace_empty_second_argument ) {
    EXPECT_EQ("hello", StringUtils::replace("hello","","a"));
}

/* Tests the method replace with empty third_argument */
TEST(StringUtils, test_method_replace_empty_third_argument ) {
	EXPECT_EQ("hello", StringUtils::replace("hello","a",""));
	EXPECT_EQ("heo", StringUtils::replace("hello","l",""));
	EXPECT_EQ("he", StringUtils::replace("hell","l",""));
	EXPECT_EQ("test", StringUtils::replace("ltestl","l",""));
}


/* Tests the method toTimeString. */
TEST(StringUtils, test_method_toTimeString) {	
	EXPECT_EQ("-00:00:01", StringUtils::toTimeString(-1));
	EXPECT_EQ("00:00:00", StringUtils::toTimeString(0));
	EXPECT_EQ("01:00:00", StringUtils::toTimeString(3600));
	EXPECT_EQ("00:00:01", StringUtils::toTimeString(1));
	EXPECT_EQ("49:40:00", StringUtils::toTimeString(178800));
	EXPECT_EQ("30883:00:01", StringUtils::toTimeString(111178801));
}

/* Tests the method escapeXML. */
TEST(StringUtils, test_method_escapeXML) {	
	std::string str;
	EXPECT_EQ("", StringUtils::escapeXML(str));
	EXPECT_EQ("test", StringUtils::escapeXML("test"))<< "nothing to be replaced.";
	EXPECT_EQ("test&apos;s", StringUtils::escapeXML("test's"))<< "' must be replaced.";
	EXPECT_EQ("1&lt;2", StringUtils::escapeXML("1<2"))<< "< must be replaced.";
	EXPECT_EQ("2&gt;1", StringUtils::escapeXML("2>1"))<< "> must be replaced.";
	EXPECT_EQ("M&amp;M", StringUtils::escapeXML("M&M"))<< "& must be replaced.";
	EXPECT_EQ("&quot;test&quot;", StringUtils::escapeXML("\"test\""))<< "\" must be replaced.";
	EXPECT_EQ("test", StringUtils::escapeXML("\01test\01"));
}
