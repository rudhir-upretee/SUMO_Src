/****************************************************************************/
/// @file    FunctionBinding.h
/// @author  Daniel Krajzewicz
/// @author  Christian Roessel
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Fri, 29.04.2005
/// @version $Id: FunctionBinding.h 13107 2012-12-02 13:57:34Z behrisch $
///
//	�missingDescription�
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef FunctionBinding_h
#define FunctionBinding_h



// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <utils/common/ValueSource.h>
#include "CastingFunctionBinding.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class FunctionBinding
 */
template< class T, typename R  >
class FunctionBinding : public ValueSource<R> {
public:
    /// Type of the function to execute.
    typedef R(T::* Operation)() const;

    FunctionBinding(T* const source, Operation operation) :
        mySource(source),
        myOperation(operation) {}

    /// Destructor.
    ~FunctionBinding() {}

    R getValue() const {
        return (mySource->*myOperation)();
    }

    ValueSource<R>* copy() const {
        return new FunctionBinding<T, R>(mySource, myOperation);
    }

    ValueSource<SUMOReal>* makeSUMORealReturningCopy() const {
        return new CastingFunctionBinding<T, SUMOReal, R>(mySource, myOperation);
    }

private:
    /// The object the action is directed to.
    T* mySource;

    /// The object's operation to perform.
    Operation myOperation;
};


#endif

/****************************************************************************/

