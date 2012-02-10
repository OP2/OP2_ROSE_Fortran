


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once
#ifndef FORTRAN_OP2_DEFINITIONS_H
#define FORTRAN_OP2_DEFINITIONS_H

#include <OP2Definitions.h>

class SgExprListExp;
class SgFunctionCallExp;

class FortranOpDatDefinition: public OpDatDefinition
{
    /*
     * ======================================================
     * Models an OP_DAT definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_DAT (OpSetName, setCardinality, data, OpDatName)
     * ======================================================
     */

  private:

    static int const index_OpSetName = 0;

    static int const index_dimension = 1;

    static int const index_data = 2;

    static int const index_OpDatName = 3;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 4;
    }

    FortranOpDatDefinition (SgExprListExp * parameters);
};

class FortranOpSetDefinition: public OpSetDefinition
{
    /*
     * ======================================================
     * Models an OP_SET definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_SET (setCardinalityName, OpSetName)
     * ======================================================
     */

  private:

    static int const index_setCardinalityName = 0;

    static int const index_OpSetName = 1;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 2;
    }

    FortranOpSetDefinition (SgExprListExp * parameters);
};

class FortranOpMapDefinition: public OpMapDefinition
{
    /*
     * ======================================================
     * Models an OP_MAP definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_MAP (source_OpSetName, destination_OpSetName, dimension, mappingCardinalityName, mappingName, OpMapName)
     * ======================================================
     */

  protected:

    std::string mappingCardinalityName;

  private:

    static int const index_sourceOpSetName = 0;

    static int const index_destinationOpSetName = 1;

    static int const index_dimension = 2;

    static int const index_mappingCardinalityName = 3;

    static int const index_mappingName = 4;

    static int const index_OpMapName = 5;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 6;
    }

    std::string const &
    getMappingCardinalityName () const
    {
      return mappingCardinalityName;
    }

    FortranOpMapDefinition (SgExprListExp * parameters);
};

class FortranOpGblDefinition: public OpGblDefinition
{
    /*
     * ======================================================
     * Models an OP_GBL definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_GBL (data, OpDatName, dimension)
     * ======================================================
     */

  private:

    static int const index_data = 0;

    static int const index_OpDatName = 1;

    static int const index_dimension = 2;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 3;
    }

    FortranOpGblDefinition (SgExprListExp * parameters);
};

class FortranOpGblScalarDefinition: public OpGblDefinition
{
    /*
     * ======================================================
     * Models a scalar OP_GBL definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_GBL (data, OpDatName)
     * ======================================================
     */

  private:

    static int const index_data = 0;

    static int const index_OpDatName = 1;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 2;
    }

    FortranOpGblScalarDefinition (SgExprListExp * parameters);
};

class FortranOpConstDefinition: public OpConstDefinition
{
    /*
     * ======================================================
     * Models an OP_CONST definition in Fortran.
     *
     * The following style is assumed:     
     * OP_DECL_CONST (OpConstName, dimension)
     * OP_DECL_CONST (OpConstName, dimension1, dimension2)
     * ======================================================
     */

  private:

    static int const index_dimension = 1;
    static int const index_dimension2 = 3;

    static int const index_OpConstName = 0;

    SgFunctionCallExp * callExpression;

  public:

    /*
     * ======================================================
     * The following function is DEPRECATED
     * A declaration of constant can have also 3 parameters
     * ======================================================
     */
    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 2;
    }

    SgFunctionCallExp *
    getCallSite ();

    FortranOpConstDefinition (SgExprListExp * parameters,
        SgFunctionCallExp * callExpression);
};

#endif
