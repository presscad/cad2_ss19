//////////////////////////////////////////////////////////////////////
// CADArxGear.cpp
// Klasse CADArxGear
// Aufgabe: Beschreibung, Berechnung und Erstellung eines Umrisses f�r ein Evolventenzahrad
//          mit der objectARX-API
//////////////////////////////////////////////////////////////////////

#include "CADArxGear.h"

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
#include <math.h>
#include <stdio.h>
#include <tchar.h>

#include <exception>
#include <string>

// Das "Epsilon" zum Testen der Bogenl�nge vor dem Speicher der Linie bzw. des Bogens:
// (Zum Testen ist das "Epsilon" relativ gro� gew�hlt.)
#define CADArx_Length_Eps  0.001

// Einige notwendige konstante Grundgr��en:
const ads_real CADArxGear::_rPi = 3.14159265359;
const ads_real CADArxGear::_rAlpha0 = 20.0 * CADArxGear::_rPi / 180.;
const ads_real CADArxGear::_rTanAlpha0 = tan(CADArxGear::_rAlpha0);
const ads_real CADArxGear::_rCosAlpha0 = cos(CADArxGear::_rAlpha0);

CADArxGear::CADArxGear(void)
{
    _bInitialized = false;
    _pPts = NULL;

    _rR0 = 0.0;
    _rRb = 0.0;
    _rRf = 0.0;
    _rRa = 0.0;
    _rDelta = 0.0;
}

CADArxGear::~CADArxGear(void)
{
    delete[] _pPts;
}

bool CADArxGear::DataInput(void)
{
    delete[] _pPts;
    _pPts = NULL;
    _bInitialized = false;
    _bIsLengthWarning = false;

    int iRet;

    // F�r den Modul, Z�hnezahl und Punktanzahl sind nur positive Zahlen erlaubt:
    acedInitGet(RSG_NONEG, NULL);
    iRet = acedGetReal(_T("\nModul: "), &_rModul);
    if ( RTNORM == iRet )
    {
        acedInitGet(RSG_NONEG, NULL);
        iRet = acedGetInt(_T("\nZ�hnezahl: "), &_nTeethNumber);
    }
    while ( RTNORM == iRet )
    {
        acedInitGet(RSG_NONEG, NULL);
        iRet = acedGetInt(_T("\nAnzahl der Punkte: "), &_nPointNumber);
        if ( RTNORM == iRet && _nPointNumber < 3 )
            acutPrintf(_T("\nEs m�ssen mindestens 3 Punkte sein! Bitte geben Sie einen Wert >= 3 ein!"));
        else
            break;
    }
    ads_point ptCenter;
    if ( RTNORM == iRet )
        iRet = acedGetPoint(NULL, _T("\nMittelpunkt (X, Y, Z): "), ptCenter);
    if ( RTNORM == iRet )
    {
        _ptCenter[X] = ptCenter[X];
        _ptCenter[Y] = ptCenter[Y];
        _ptCenter[Z] = ptCenter[Z];
        _bInitialized = true;
    }

    return _bInitialized;
}

// from https://stackoverflow.com/questions/10737644/convert-const-char-to-wstring
std::wstring s2ws(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

//prints std::string converted to wstring in autocad
void printMessage(std::string const& s)
{
    acutPrintf(s2ws(s).c_str());
}

bool CADArxGear::Calc(void)
{
    if ( !_bInitialized )
        return false;

    // F�gen Sie hier den Code zur Berechnung der _nPointNumber Flankenpunkte ein
    // und speichern Sie die Daten in einem entsprechenden _pPts-Array.

    _rR0 = (_rModul * _nTeethNumber) / 2;
    _rRb = _rR0 * _rCosAlpha0;
    _rRf = _rR0 - _rModul;
    _rRa = _rR0 + _rModul;
    _rDelta = _rPi / (2 * _nTeethNumber) + _rTanAlpha0 - _rAlpha0;
//    _rPhi = 2 * pi / _nTeethNumber;

    // fusskreis r_f > flankenkreis r_b
    if (_rRf < _rRb)
    {
        printMessage("CADArxGear::Calc:: fusskreis r_f < flankenkreis r_b\n");
        throw(std::runtime_error("CADArxGear::Calc:: fusskreis r_f < flankenkreis r_b\n"));
    }

	// Zum Testen und f�r die Bewertung die berechneten Zwischenergebnisse ausgeben:
	acutPrintf(_T("\n r0 = %6.2f"    ), _rR0);
	acutPrintf(_T("\n rb = %10.6f"   ), _rRb);
	acutPrintf(_T("\n rf = %6.2f"    ), _rRf);
	acutPrintf(_T("\n ra = %6.2f"    ), _rRa);
	acutPrintf(_T("\n delta = %10.6f"), _rDelta);

    if (_pPts != NULL)
    {
        delete[] _pPts;
    }

    _pPts = new AcGePoint2d[_nPointNumber];

    double r = _rRb;
    double dR = (_rRa - _rRb) / _nPointNumber;
    for (size_t i = 0; i < _nPointNumber; i++)
    {
        double u = 1 / _rRb * sqrt(r * r - _rRb * _rRb);

        double Delta = u - _rDelta;
        double cosDelta = cos(Delta);
        double sinDelta = cos(Delta);
        double ptX = _rRb * (cosDelta + u * sinDelta);
        double ptY = _rRb * (sinDelta - u * cosDelta);

        // create point which is moved into place
        _pPts[i] = AcGePoint2d(ptX + _ptCenter.x, ptY + _ptCenter.y);

        r += dR;
    }

    return true;
}

void CADArxGear::CreateLine(AcDbBlockTableRecord* pBlockTableRecord,
                            const AcGePoint2d&    ptStart,
                            const AcGePoint2d&    ptEnd)
{
    // F�gen Sie hier den Code zum Speichern einer Linie ein
    // und zum Pr�fen der L�nge vor dem Speichern.

    //TODO: check bogenl�nge for large enough length

    // check if line is long enough
    auto start = AcGePoint3d(ptStart.x, ptStart.y, _ptCenter.z);
    auto end = AcGePoint3d(ptEnd.x, ptEnd.y, _ptCenter.z);

    AcDbLine* pEntity = new AcDbLine(start, end);
    double length = (start.x - end.x) * (start.x - end.x);
    length += (start.y - end.y) * (start.y - end.y);
//    length += (start.z - end.z) * (start.z - end.z);
    if (sqrt(length) < CADArx_Length_Eps)
    {
        if (!_bIsLengthWarning)
        {
            printMessage("a length is less than 1!\n");
        }
        _bIsLengthWarning = true;
    }

    AcDbObjectId pOutputId; // to give as reference and thereafter ignore
    Acad::ErrorStatus es = pBlockTableRecord->appendAcDbEntity(pOutputId, pEntity);

    pEntity->close();

    if (es != Acad::ErrorStatus::eOk || _bIsLengthWarning)
    {
        throw(std::runtime_error("CADarxLetter:: an error occured, i can not continue!"));
    }

}

void CADArxGear::CreateArc(AcDbBlockTableRecord* pBlockTableRecord,
                           const AcGePoint2d&    ptStart,
                           const AcGePoint2d&    ptEnd,
                           ads_real              rR)
{
    // F�gen Sie hier den Code zum Speichern eines Bogenst�cks ein
    // und zum Pr�fen der Bogenl�nge vor dem Speichern.

    //TODO: check bogenl�nge for large enough length

    double startAngle = asin((ptStart.y - _ptCenter.y)/rR), endAngle = asin((ptEnd.y - _ptCenter.y) / rR);
    //TODO: confirm correct order of arguments
    AcDbArc* pEntity = new AcDbArc(_ptCenter, rR, startAngle, endAngle);

    // bogenl�nge ist r*alpha = radius * �ffnungswinkel
    // check if radius is large enough
    if (rR * (endAngle - startAngle) < CADArx_Length_Eps)
    {
        if (!_bIsLengthWarning)
        {
            printMessage("a radius is less than 1!\n");
        }
        _bIsLengthWarning = true;
    }
    AcDbObjectId pOutputId; // to give as reference and thereafter ignore
    Acad::ErrorStatus es = pBlockTableRecord->appendAcDbEntity(pOutputId, pEntity);

    pEntity->close();

    if (es != Acad::ErrorStatus::eOk || _bIsLengthWarning)
    {
        throw(std::runtime_error("CADarxLetter:: an error occured, i can not continue!"));
    }

}

void CADArxGear::Create(void)
{
    if ( NULL == _pPts )
        return;

    // F�gen Sie hier den Code zur Speicherung der AcDbLine- bzw. AcDbArc-Elemente ein.
    // Zur Ermittlung der Punkte der jeweils gedrehten Flanke die Methode k�nnen Sie
    // die Methode AcGePoint2d::rotateBy. Informieren Sie sich in der Online-Hilfe �ber
    // diese Methode.
}

