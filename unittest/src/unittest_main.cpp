/****************************************************************************/
/// @file    unittest_main.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    Sept 2009
/// @version $Id: unittest_main.cpp 11671 2012-01-07 20:14:30Z behrisch $
///
// main for the unit tests
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
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>
#include <gtest/gtest.h>


// ===========================================================================
// method definitions
// ===========================================================================
int main(int argc, char **argv) {
  std::cout << "Running unit tests\n";

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS(); 
}
