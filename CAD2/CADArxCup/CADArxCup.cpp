//////////////////////////////////////////////////////////////////////
// CADArxCup.cpp
// Klasse CADArxCup
// Aufgabe: Beschreibung, Berechnung und Erstellung eines Pokals
//          mit der objectARX-API
//////////////////////////////////////////////////////////////////////

#include "CADArxCup.h"

// Damit mit Warnungsstufe 4 �bersetzt werden kann:
#pragma warning( disable : 4100 )
#pragma warning( disable : 4201 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4512 )

#include <aced.h>
#include <dbents.h>
#include <dbregion.h>
#include <dbsol3d.h>
#include <dbgroup.h>
#include <dbapserv.h>
#include <adscodes.h>
#include <acedads.h>
#include <math.h>
#include <stdio.h>
#include <tchar.h>

// test draw lines
#include <stdexcept>
#include <dbsymtb.h>




// Das "Epsilon" zum Testen der Dicke:
#define CADArx_Width_Eps  0.01

const ads_real CADArxCup::_rPi = 3.14159265359;

CADArxCup::CADArxCup(void)
{
    _bInitialized = false;

	// Diese Werte werden nur wegen der Testausgabe initialisiert.
	_rHorWidth = 0.0;
	_rSphereRadiusOnCup = 0.0;
    _rInnerStemHeight = 0.0;

	// zum testweise anzeigen der kontur
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(_pBlockTable, AcDb::kForRead);
	_pBlockTable->getAt(ACDB_MODEL_SPACE, _pBlockTableRecord, AcDb::kForWrite);
}

CADArxCup::~CADArxCup(void)
{
	_pBlockTableRecord->close();
	_pBlockTable->close();
}

bool CADArxCup::DataInput(void)
{
    _bInitialized = false;

    int iRet;

    // F�r den Durchmesser, H�he, Dicke und Kugelradius sind nur positive Zahlen erlaubt:
    acedInitGet(RSG_NONULL,NULL);
    iRet = acedGetReal(_T("\ngo�er Durchmesser: "),&_rDiameter);
    if ( RTNORM == iRet )
    {
        acedInitGet(RSG_NONULL,NULL);
        iRet = acedGetReal(_T("\nGesamth�he: "),&_rHeight);
    }
    if ( RTNORM == iRet )
    {
        acedInitGet(RSG_NONULL,NULL);
        iRet = acedGetReal(_T("\nDicke: "),&_rWidth);
    }
    if ( RTNORM == iRet )
    {
        acedInitGet(RSG_NONEG,NULL);
        iRet = acedGetInt(_T("\nAnzahl der Kugeln: "),&_nSphereNumber);
    }
    if ( RTNORM == iRet && _nSphereNumber > 0 )
	{
        acedInitGet(RSG_NONULL,NULL);
        iRet = acedGetReal(_T("\nKugelradius: "),&_rRadiusSphere);
	}

    ads_point ptRef;
    if ( RTNORM == iRet )
        iRet = acedGetPoint(NULL, _T("\nBezugspunkt (X,Y,Z): "), ptRef);
    if ( RTNORM == iRet )
    {
        _ptRef[X] = ptRef[X];
        _ptRef[Y] = ptRef[Y];
        _ptRef[Z] = ptRef[Z];
        _bInitialized = true;
    }

    return _bInitialized;
}

bool CADArxCup::Calc(void)
{
    if ( !_bInitialized )
        return false;

    // F�gen Sie hier den Code zur Pr�fung der Eingabedaten sowie
    // zur Berechnung der Zwischenergebnisse und des Ausgangsprofils
    // f�r den Rotationsk�rper ein.
    // Zwischenergebnisse: _rHorWidth, _rSphereRadiusOnCup (siehe CADArxCup.h)
    // AcGePoint2d-Feld f�r das Profil: _pPtsProfile[10] (siehe CADArxCup.h)

	if (_rWidth < CADArx_Width_Eps)
	{
		acutPrintf(_T("\n_rWidth too small : %8.6f"), _rWidth);
		return false;
	}
	if (_rHeight < 16 * _rWidth)
	{
		acutPrintf(_T("\n_rHeight too small : %8.6f"), _rHeight);
		return false;
	}
	if (_rDiameter < 16 * _rWidth)
	{
		acutPrintf(_T("\n_rDiameter too small : %8.6f"), _rDiameter);
		return false;
	}


    _pPtsProfile[0] = _pPtsProfile[9] = AcGePoint2d();
    _pPtsProfile[1] = AcGePoint2d( _rDiameter * 3.0f / 8.0f, 0.0f);
    _pPtsProfile[2] = AcGePoint2d( _rDiameter / 8.0f, _rHeight / 8.0f );
    _pPtsProfile[3] = AcGePoint2d( _rDiameter / 8.0f, _rHeight / 4.0f );
    _pPtsProfile[4] = AcGePoint2d( _rDiameter / 2.0f, _rHeight );

	AcGeVector2d a((_pPtsProfile[2] - _pPtsProfile[3]).x, (_pPtsProfile[2] - _pPtsProfile[3]).y);
	AcGeVector2d b((_pPtsProfile[4] - _pPtsProfile[3]).x, (_pPtsProfile[4] - _pPtsProfile[3]).y);

	double dot = a.dotProduct(b);
	dot /= (a.length() * b.length());
	double angle = a.angleTo(b);
	double alpha = acos(dot)/2;
	alpha = angle/2;
	double tanAlpha = tan(alpha);
	double gegenkathete = _rWidth / tanAlpha;


	_rInnerStemHeight = _pPtsProfile[3].y + gegenkathete;

	double cosW = cos(_rPi - 2 * alpha);
	_rHorWidth = _rWidth / cosW;



    _pPtsProfile[5] = AcGePoint2d( _rDiameter / 2.0f - _rHorWidth, _rHeight);
    _pPtsProfile[6] = AcGePoint2d( _rDiameter / 8.0f - _rWidth, _rInnerStemHeight);
    _pPtsProfile[7] = AcGePoint2d( _rDiameter / 8.0f - _rWidth, _rHeight / 8.0f);
    _pPtsProfile[8] = AcGePoint2d( 0.0f, _rHeight / 8.0f);

	//TODO: calculate rSphereRadiusOnCup
	double andererWinkel = _rPi - angle;
	double ankathete = abs(_rHeight -_rInnerStemHeight -_rHeight/8.0f);
	_rSphereRadiusOnCup = tan(andererWinkel) * ankathete  +  _rDiameter/8.0f - _rHorWidth/2.0f;
	if (false)
	{
		if (_rSphereRadiusOnCup < _rWidth + CADArx_Width_Eps)
		{
			acutPrintf(_T("\n_rSphereRadiusOnCup too small : %8.6f"), _rSphereRadiusOnCup);
			return false;
		}
		if (_rHeight / 8 - _rWidth < _rSphereRadiusOnCup)
		{
			acutPrintf(_T("\n_rSphereRadiusOnCup too big : %8.6f"), _rSphereRadiusOnCup);
			return false;
		}

	}

	// Zum Testen und f�r die Bewertung
    // die berechneten Werte ausgeben:
	acutPrintf(_T("\na.x : %8.6f"), a.x);
	acutPrintf(_T(" a.y : %8.6f"), a.y);
	acutPrintf(_T("\nb.x : %8.6f"), b.x);
	acutPrintf(_T(" b.y : %8.6f"),  b.y);
	acutPrintf(_T("\nangle                : %8.6f"), angle);
	acutPrintf(_T("\ndot                  : %8.6f"), dot);
	acutPrintf(_T("\ngegenkathete         : %8.6f"), gegenkathete);
	acutPrintf(_T("\nankathete            : %8.6f"), ankathete);
	acutPrintf(_T("\nalpha                : %8.6f"), alpha);
	acutPrintf(_T("\ncosW                 : %8.6f"), cosW);
	acutPrintf(_T("\n_rDiameter           : %8.6f"), _rDiameter);
	acutPrintf(_T("\n_rWidth              : %8.6f"), _rWidth);
	acutPrintf(_T("\n_rHeight             : %8.6f"), _rHeight);
	acutPrintf(_T("\nHorizontal Width     : %8.6f"), _rHorWidth);
	acutPrintf(_T("\nSphere Radius on Cup : %8.6f"), _rSphereRadiusOnCup);
	acutPrintf(_T("\nInner Stem Height    : %8.6f"), _rInnerStemHeight);


    return true;
}

AcDb3dSolid* CADArxCup::CreateSphere(double dPhi)
{
    // F�gen Sie hier den Code zum Erzeugen einer Kugel ein.
	AcDb3dSolid* sp = new AcDb3dSolid();
	if (sp->createSphere(dPhi) != Acad::ErrorStatus::eOk)
	{
		acutPrintf(_T("\n gotten to line: %i"), __LINE__);
		delete[] sp;
		return NULL;
	}
    return sp;
}

void CADArxCup::Create(void)
{
    // Fuegen Sie hier dn Code zur Erzeugung und Speicherung des Pokals ein.
    // Achten Sie darauf, dass f�r alle erzeugten Elemente (die Linien,
    // die Fl�che, ...) die close-Methode aufgerufen wird.

	// testweise kontur anzeigen
	//for (size_t i = 0; i < 8; i++)
	//{
	//	CreateLine(_pBlockTableRecord, _pPtsProfile[i], _pPtsProfile[i + 1]);
	//}

	AcDbVoidPtrArray lines(9);
	for (size_t i = 0; i < 9; i++)
	{
		AcDbLine* currentLine = new AcDbLine(AcGePoint3d(_pPtsProfile[i    ].x + _ptRef.x, _ptRef.y, _pPtsProfile[i    ].y + _ptRef.z),
                                             AcGePoint3d(_pPtsProfile[i + 1].x + _ptRef.x, _ptRef.y, _pPtsProfile[i + 1].y + _ptRef.z));

		//// visualize lines to revolve
		//if (false)
		//{
		//	AcDbObjectId pOutputId; // to give as reference and thereafter ignore
		//	Acad::ErrorStatus es = _pBlockTableRecord->appendAcDbEntity(pOutputId, currentLine);
		//	if (es != Acad::ErrorStatus::eOk)
		//	{
		//		acutPrintf(_T("\n gotten to line: %i"), __LINE__);
		//		return;
		//	}
		//	currentLine->close();
		//
		//	currentLine = new AcDbLine(AcGePoint3d(_pPtsProfile[i].x, 0, _pPtsProfile[i].y),
		//		AcGePoint3d(_pPtsProfile[i + 1].x, 0, _pPtsProfile[i + 1].y));
		//}

		lines.append(currentLine);
	}

	AcDbVoidPtrArray regions(1);
	Acad::ErrorStatus es1 = AcDbRegion::createFromCurves(lines, regions);
	if (es1 != Acad::ErrorStatus::eOk)
	{
		acutPrintf(_T("\n gotten to line: %i"), __LINE__);
		return;
	}
	AcGeVector3d up(0,0,1);

	AcDb3dSolid* cup = new AcDb3dSolid();
	Acad::ErrorStatus es2 = cup->revolve((AcDbRegion*)regions[0], _ptRef, up, 2 * _rPi);

	if (es2 != Acad::ErrorStatus::eOk)
	{
		acutPrintf(_T("\n gotten to line: %i"), __LINE__);
		return;
	}

	{
		AcDbObjectId pOutputId; // to give as reference and thereafter ignore
		Acad::ErrorStatus es3 = _pBlockTableRecord->appendAcDbEntity(pOutputId, cup);
		if (es3 != Acad::ErrorStatus::eOk)
		{
			acutPrintf(_T("\n gotten to line: %i"), __LINE__);
			return;
		}
	}

	// create sphere(s)
	for (size_t i = 0; i < _nSphereNumber; i++)
	{
		double angle = i * 2 * _rPi / _nSphereNumber;
		double x = cos(angle)*_rSphereRadiusOnCup;
		double y = sin(angle)*_rSphereRadiusOnCup;
		AcDb3dSolid* sphere = CreateSphere(_rRadiusSphere);
		AcGeVector3d ort = AcGeVector3d(_ptRef.x + x, _ptRef.y + y,_rHeight*7/8 + _ptRef.z);
		AcGeMatrix3d mat(ort);
		sphere->transformBy(mat);

		auto es4 = cup->booleanOper(AcDb::kBoolUnite, sphere);
		if (es4 != Acad::ErrorStatus::eOk)
		{
			acutPrintf(_T("\n gotten to line: %i"), __LINE__);
			return;
		}
	}
	cup->close();
}


// stolen from gear
void CADArxCup::CreateLine(AcDbBlockTableRecord* pBlockTableRecord,
	const AcGePoint2d&    ptStart,
	const AcGePoint2d&    ptEnd)
{
	// F�gen Sie hier den Code zum Speichern einer Linie ein
	// und zum Pr�fen der L�nge vor dem Speichern.

	//TODO: check bogenl�nge for large enough length

	// check if line is long enough
	AcGePoint3d start(ptStart.x , ptStart.y, 0);
	AcGePoint3d end(ptEnd.x , ptEnd.y, 0);

	AcDbLine* pEntity = new AcDbLine(start, end);

	AcDbObjectId pOutputId; // to give as reference and thereafter ignore
	Acad::ErrorStatus es = pBlockTableRecord->appendAcDbEntity(pOutputId, pEntity);

	pEntity->close();

	if (es != Acad::ErrorStatus::eOk)
	{
		throw(std::runtime_error("CADarxLetter:: an error occured, i can not continue!"));
	}
}