//////////////////////////////////////////////////////////////////////
// CADArxLetter.cpp
// Klasse CADArxLetter
// Aufgabe: Beschreibung, Berechnung und Erstellung eines Evolventenzahrades
//          mit der objectARX-API
//////////////////////////////////////////////////////////////////////

#include "CADArxLetter.h"

// Damit mit Warnungsstufe 4 �bersetzt werden kann:
#pragma warning( disable : 4100 )
#pragma warning( disable : 4201 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4512 )

#include <aced.h>
#include <dbents.h>
#include <dbgroup.h>
#include <dbapserv.h>
#include <adscodes.h>
#include <acedads.h>

//#define _USE_MATH_DEFINES
#include <math.h> // has M_PI if above is defined
#include <stdio.h>
#include <tchar.h>

#include <dbsymtb.h>



// Zum Testen ist das "Epsilon" relativ gro� gew�hlt.
#define CADArx_Length_Eps  0.001

const ads_real CADArxLetter::_rPi = /* M_PI; //*/ 3.14159265359;

CADArxLetter::CADArxLetter(void)
{
    _bInitialized = false;
}

CADArxLetter::~CADArxLetter(void)
{
}

bool CADArxLetter::DataInput(void)
{
    _bInitialized = false;

    int iRet;

    acedInitGet(RSG_NONEG, NULL);
    iRet = acedGetReal(_T("\nBreite: "), &_rWidth);
    if ( RTNORM == iRet )
    {
        acedInitGet(RSG_NONEG, NULL);
        iRet = acedGetReal(_T("\nH�he: "), &_rHeight);
    }
    if ( RTNORM == iRet )
    {
        acedInitGet(RSG_NONEG, NULL);
        iRet = acedGetReal(_T("\nAbstand: "), &_rDist);
    }
    ads_point ptRef;
    if (RTNORM == iRet)
    {
        iRet = acedGetPoint(NULL, _T("\nLinke untere Ecke (X, Y, Z): "), ptRef);
    }
    if (RTNORM == iRet)
    {
        _ptRef[X] = ptRef[X];
        _ptRef[Y] = ptRef[Y];
        _ptRef[Z] = ptRef[Z];
        _bInitialized = true;
    }

    return _bInitialized;
}

void CADArxLetter::Create(void)
{
    if (!_bInitialized)
    {
        return;
    }

    // F�gen Sie hier den Code zur Berechnung Speicherung
	// der AcDbLine- bzw. AcDbArc-Elemente ein.

    AcGePoint3d lu = _ptRef;
    lu.y += _rHeight;

    // lower left to upper left
    AcDbLine foo(_ptRef, lu);

    AcDbBlockTableRecord r;
    r.appendAcDbEntity(&foo);
}

