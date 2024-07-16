

#pragma hdrstop

#include "ExportCIMXML.h"

#include "Sysutils.h"
#include "Utilities.h"
#include "Circuit.h"
#include "DSSClassDefs.h"
#include "DSSGlobals.h"
#include "CktElement.h"
#include "PDElement.h"
#include "PCElement.h"
#include "generator.h"
#include "Load.h"
#include "RegControl.h"
#include "VSource.h"
#include "Line.h"
#include "Transformer.h"
#include "Ucomplex.h"
#include "Ucmatrix.h"
#include "LineCode.h"
#include "fuse.h"
#include "Capacitor.h"
#include "CapControl.h"
#include "CapControlVars.h"
#include "Reactor.h"
#include "Feeder.h"
#include "ConductorData.h"
#include "LineUnits.h"
#include "LineGeometry.h"
#include <math.h>
#include "HashList.h"
#include "WireData.h"
#include "XfmrCode.h"
#include "LineSpacing.h"
#include "CableData.h"
#include "CNData.h"
#include "TSData.h"
#include "Storage.h"
#include "PVsystem.h"
#include "Relay.h"
#include "Recloser.h"
#include "AutoTrans.h"
#include "InvControl.h"
#include "ExpControl.h"
#include "DSSObject.h"
#include "DSSClass.h"

#include "System.h"
#include <algorithm>


class TBankObject;
class TFileDealer;
class TOpLimitObject;




enum UuidChoice {Bank,
                 Wdg,
                 XfCore,
                 XfMesh,
                 WdgInf,
                 ScTest,
                 OcTest,
                 BaseV,
                 LinePhase,
                 LoadPhase,
                 GenPhase,
                 CapPhase,
                 SolarPhase,
                 BatteryPhase,
                 XfLoc,
                 LoadLoc,
                 LineLoc,
                 CapLoc,
                 Topo,
                 ReacLoc,
                 SolarLoc,
                 BatteryLoc,
                 OpLimV,
                 OpLimI,
                 LoadResp,
                 CIMVer,
                 PosPt,
                 CoordSys,
                 TopoIsland,
                 Station,
                 GeoRgn,
                 SubGeoRgn,
                 ZData,
                 OpLimT,
                 XfInfo,
                 FdrLoc,
                 OpLimAHi,
                 OpLimALo,
                 OpLimBHi,
                 OpLimBLo,
                 MachLoc,
                 PVPanels,
                 Battery,
                 SrcLoc,
                 TankInfo,
                 TankAsset,
                 TapInfo,
                 TapCtrl,
                 TapAsset,
                 PUZ,
                 WirePos,
                 NormAmps,
                 EmergAmps,
                 I1547NameplateData,
                 I1547NameplateDataApplied,
                 I1547Signal,
                 I1547VoltVar,
                 I1547WattVar,
                 I1547ConstPF,
                 I1547VoltWatt,
                 I1547ConstQ };
enum ProfileChoice {FunPrf,
                    EpPrf,
                    GeoPrf,
                    TopoPrf,
                    CatPrf,
                    SshPrf,
                    DynPrf };

class TBankObject: public TNamedObject {
  typedef TNamedObject inherited;
  public:
  String vectorGroup;
  int maxWindings;
  int nWindings;
  int* connections;
  bool bAuto;
  int* angles;
  int* phaseA;
  int* phaseB;
  int* phaseC;
  int* ground;
  TPDElement pd_unit;  // save this for writing the bank coordinates
  TBankObject( int MaxWdg );
  ~TBankObject( );
  void AddTransformer( TTransfObj pXf );
  void AddAutoTransformer( TAutoTransObj pAuto );
  void BuildVectorGroup( );
};


class TOpLimitObject: public TNamedObject {
  typedef TNamedObject inherited;
  public:
  double NormAmps;
  double EmergAmps;
  TOpLimitObject( double norm, double emerg );
  ~TOpLimitObject( );
};


class TFileDealer: public TObject {
  typedef TObject inherited;
  private:
    // the Combined XML can be broken into seven separate profiles
  TTextRec F_FUN;
  TTextRec F_EP;
  TTextRec F_SSH;
  TTextRec F_CAT;
  TTextRec F_GEO;
  TTextRec F_TOPO;
  TTextRec F_DYN;
  String roots [ 7 /*# ProfileChoice */ ];
  TUuid ids [ 7 /*# ProfileChoice */ ];
  public:
  bool Separate;
  TFileDealer( bool Combined, String Filename );
  ~TFileDealer( );
  void WriteCimLn( ProfileChoice prf, const String S );
  void StartInstance( ProfileChoice prf, String Root, TNamedObject Obj );
  void StartFreeInstance( ProfileChoice prf, String Root, TUuid uuid );
  void EndInstance( ProfileChoice prf, String Root );
};



//THashList* UuidHash;       // index is 1-based

TUuid* UuidList;  // index is 0-based

String* UuidKeyList;
THashList BankHash;
TBankObject** BankList;
THashList OpLimitHash;
TOpLimitObject** OpLimitList;
TFileDealer* FD = NULL;
//  CIM_NS = 'http://iec.ch/TC57/2012/CIM-schema-cim17';



const char CIM_NS[] = "http://iec.ch/TC57/CIM100";

void StartCIMFile( Textfile& F, String Filenm, ProfileChoice prf );
/*# forward */


void TFileDealer::WriteCimLn( ProfileChoice prf, const String S )
{
  /*if (Separate)
  {
    if ( prf != FunPrf )
    {
      if ( roots[prf].size() ) < 1 )
      {
        StartFreeInstance( prf, roots[FunPrf], ids[FunPrf] );
      }
    }
    switch ( prf )
    {
      case FunPrf:
        WriteLn( F_FUN, S );
      break;
      case EpPrf:
        WriteLn( F_EP, S );
      break;
      case GeoPrf:
        WriteLn( F_GEO, S );
      break;
      case TopoPrf:
        WriteLn( F_TOPO, S );
      break;
      case CatPrf:
        WriteLn( F_CAT, S );
      break;
      case SshPrf:
        WriteLn( F_SSH, S );
      break;
      case DynPrf:
        WriteLn( F_DYN, S );
      break;
    }
  }
  else
  {
    WriteLn( F_FUN, S );
  }*/
}

TFileDealer::TFileDealer( bool Combined, String Filename )
 : Separate(! Combined)
{
  ProfileChoice i = FunPrf;
  // inherited::Create();
  if ( Separate )
  {
    for ( int stop = /*# High(ProfileChoice) */ 6, i = /*# Low(ProfileChoice) */ 0; i <= stop; i++ )
      roots[i] = "";
    StartCIMFile( F_FUN, Filename + "_FUN.XML", FunPrf );
    StartCIMFile( F_GEO, Filename + "_GEO.XML", GeoPrf );
    StartCIMFile( F_TOPO, Filename + "_TOPO.XML", TopoPrf );
    StartCIMFile( F_SSH, Filename + "_SSH.XML", SshPrf );
    StartCIMFile( F_CAT, Filename + "_CAT.XML", CatPrf );
    StartCIMFile( F_EP, Filename + "_EP.XML", EpPrf );
    StartCIMFile( F_DYN, Filename + "_DYN.XML", EpPrf );
  }
  else
  {
    StartCIMFile( F_FUN, Filename, FunPrf );
  }
}


void TFileDealer::StartInstance( ProfileChoice prf, String Root, TNamedObject Obj )
{
/*  if (Separate)
  { // must be first to avoid stack overflow in WriteCimLn
    roots[prf] = Root;
    ids[prf] = Obj.uuid;
  }
  WriteCimLn( prf, Format( "<cim:%s rdf:ID=\"%s\">", VECTOROFCONST(( Root, Obj.CIM_ID )) ) );
  WriteCimLn( prf, Format( "  <cim:IdentifiedObject.mRID>%s</cim:IdentifiedObject.mRID>", VECTOROFCONST(( Obj.CIM_ID )) ) );
  WriteCimLn( prf, Format( "  <cim:IdentifiedObject.name>%s</cim:IdentifiedObject.name>", VECTOROFCONST(( Obj.LocalName )) ) );
  */
}


void TFileDealer::StartFreeInstance( ProfileChoice prf, String Root, TUuid uuid )
{
  /*
  if ( Separate )
  { // must be first to avoid stack overflow in WriteCimLn
    roots[prf] = Root;
    ids[prf] = uuid;
  }
  WriteCimLn( prf, Format( "<cim:%s rdf:ID=\"%s\">", VECTOROFCONST(( Root, UUIDToCIMString( uuid ) )) ) );
  */
}

void ExportCDPSM(String Filenm, String SUBSTATION, String SubGeographicRegion, String GeographicRegion, TUuid FdrUUID, TUuid SubUUID, TUuid SubGeoUUID, TUuid RgnUUID, bool Combined)
{
    //TBD
}

void TFileDealer::EndInstance( ProfileChoice prf, String Root )
{
  /*
  ProfileChoice i = FunPrf;
  if ( Separate )
  {
    for ( int stop = 6, i = 0; i <= stop; Inc(i) )
    {
      if ( roots[i].size() ) > 0 )
      {
        WriteCimLn( i, Format( "</cim:%s>", VECTOROFCONST(( Root )) ) );
        roots[i] = "";
      }
    }
  }
  else
    WriteCimLn( prf, Format( "</cim:%s>", VECTOROFCONST(( Root )) ) );
  */
}


TFileDealer::~TFileDealer( )
{
  WriteLn( F_FUN, "</rdf:RDF>" );
  CloseFile( F_FUN );
  if ( Separate )
  {
    WriteLn( F_GEO, "</rdf:RDF>" );
    WriteLn( F_CAT, "</rdf:RDF>" );
    WriteLn( F_SSH, "</rdf:RDF>" );
    WriteLn( F_TOPO, "</rdf:RDF>" );
    WriteLn( F_EP, "</rdf:RDF>" );
    WriteLn( F_DYN, "</rdf:RDF>" );
    CloseFile( F_GEO );
    CloseFile( F_CAT );
    CloseFile( F_SSH );
    CloseFile( F_TOPO );
    CloseFile( F_EP );
    CloseFile( F_DYN );
  }
  // todo check:  inherited::Destroy;
}

void ParseSwitchClass( TLineObj pLine, String& swtCls, double& ratedAmps, double& breakingAmps )
{
/*  TFuseObj pFuse;
  TRelayObj pRelay;
  TRecloserObj pRecloser;
  swtCls = "LoadBreakSwitch";
  ratedAmps = pLine.NormAmps;
  breakingAmps = ratedAmps;
  pFuse = ActiveCircuit[ActiveActor]->Fuses.Get_First();
  while ( pFuse != NULL )
  {
    if ( pFuse.ControlledElement == pLine )
    {
      swtCls = "Fuse";
      ratedAmps = pFuse.RatedCurrent;
      breakingAmps = 0.0;
      return;
    }
    pFuse = ActiveCircuit[ActiveActor]->Fuses.Get_Next();
  }
  pRelay = ActiveCircuit[ActiveActor]->Relays.Get_First();
  while ( pRelay != NULL )
  {
    if ( pRelay.ControlledElement == pLine )
    {
      swtCls = "Breaker";
      return;
    }
    pRelay = ActiveCircuit[ActiveActor]->Relays.Get_Next();
  }
  pRecloser = ActiveCircuit[ActiveActor]->Reclosers.Get_First();
  while ( pRecloser != NULL )
  {
    if ( pRecloser.ControlledElement == pLine )
    {
      swtCls = "Recloser";
      return;
    }
    pRecloser = ActiveCircuit[ActiveActor]->Reclosers.Get_Next();
  }
  */
}

// this returns s1, s2, or a combination of ABCN



String PhaseString( TDSSCktElement pElem, int Bus, bool bAllowSec = true ) // if order doesn't matter

{/*
  String result;
  String val, phs;
  int dot = 0;
  bool bSec = false;
  phs = pElem.FirstBus;
  for ( int stop = Bus, dot = 2; dot <= stop; dot++)
    phs = pElem.NextBus;
  bSec = false;
  if ( bAllowSec )
  {
    if ( pElem.NPhases == 2 )
      if ( ActiveCircuit[ActiveActor]->Buses[pElem.Terminals[Bus].BusRef - 1]->kVBase < 0.25 )
        bSec = true;
    if ( pElem.NPhases == 1 )
      if ( ActiveCircuit[ActiveActor]->Buses[pElem.Terminals[Bus].BusRef - 1]->kVBase < 0.13 )
        bSec = true;
  }
  dot = phs.find( "." );
  if ( dot < 1 )
  {
    val = "ABC";
  }
  else
  {
    phs = phs.substr( dot + 1, phs.size() ) );
    if ( phs.find( "3" ) > 0 )
      bSec = false; // i.e. it's a three-phase secondary, not split-phase
    if ( bSec )
    {
      if ( phs.find( "1" ) > 0 )
      {
        val = "s1";
        if ( phs.find( "2" ) > 0 )
          val = val + "2";
      }
      else
        if ( phs.find( "2" ) > 0 )
          val = "s2";
    }
    else
    {
      val = "";
      if ( phs.find( "1" ) > 0 )
        val = val + "A";
      if ( phs.find( "2" ) > 0 )
        val = val + "B";
      if ( phs.find( "3" ) > 0 )
        val = val + "C";
      if ( phs.find( "4" ) > 0 )
        val = val + "N";
    }
  }


  result = val;
  return result;
    */
  return "";// alterante result
}

// returns s1, s12, s2, or an ordered combination of ABC



String PhaseOrderString( TDSSCktElement pElem, int Bus, bool bAllowSec = true ) // for transposition

{
/*
  String result;
  String phs;
  int dot = 0;
  bool bSec = false;
  phs = pElem.FirstBus;
  for ( int stop = Bus, dot = 2; dot <= stop; dot++)
    phs = pElem.NextBus;
  bSec = false;
  if ( bAllowSec )
  {
    if ( pElem.NPhases == 2 )
      if ( ActiveCircuit[ActiveActor]->Buses[pElem.Terminals[Bus].BusRef - 1]->kVBase < 0.25 )
        bSec = true;
    if ( pElem.NPhases == 1 )
      if ( ActiveCircuit[ActiveActor]->Buses[pElem.Terminals[Bus].BusRef - 1]->kVBase < 0.13 )
        bSec = true;
  }
  dot = phs.find( "." );
  if ( dot < 1 )
  {
    result = "ABC";
  }
  else
  {
    phs = phs.substr( dot + 1, phs.size() ) );
    if ( phs.find( "3" ) > 0 )
      bSec = false; // i.e. it's a three-phase secondary, not split-phase
    if ( bSec )
    {
      if ( phs.find( "1" ) > 0 )
      {
        result = "s1";
        if ( phs.find( "2" ) > 0 )
          result = result + "2";
      }
      else
        if ( phs.find( "2" ) > 0 )
          result = "s2";
    }
    else
      if ( phs.find( "1.2.3" ) > 0 )
        result = "ABC";
      else
        if ( phs.find( "1.3.2" ) > 0 )
          result = "ACB";
        else
          if ( phs.find( "2.3.1" ) > 0 )
            result = "BCA";
          else
            if ( phs.find( "2.1.3" ) > 0 )
              result = "BAC";
            else
              if ( phs.find( "3.2.1" ) > 0 )
                result = "CBA";
              else
                if ( phs.find( "3.1.2" ) > 0 )
                  result = "CAB";
                else
                  if ( phs.find( "1.2" ) > 0 )
                    result = "AB";
                  else
                    if ( phs.find( "1.3" ) > 0 )
                      result = "AC";
                    else
                      if ( phs.find( "2.3" ) > 0 )
                        result = "BC";
                      else
                        if ( phs.find( "2.1" ) > 0 )
                          result = "BA";
                        else
                          if ( phs.find( "3.2" ) > 0 )
                            result = "CB";
                          else
                            if ( phs.find( "3.1" ) > 0 )
                              result = "CA";
                            else
                              if ( phs.find( "1" ) > 0 )
                                result = "A";
                              else
                                if ( phs.find( "2" ) > 0 )
                                  result = "B";
                                else
                                  result = "C";
  }
  return result;
  */
      return ""; //alternative
}


String DeltaPhaseString( TDSSCktElement pElem )
{
  String result = "";
  String phs;
  int dot = 0;
  phs = pElem.Get_FirstBus();
  dot = phs.find( "." );
  if ( ( dot < 1 ) || ( pElem.Get_NPhases() == 3 ) )
  {
    result = "ABC"; // if Nphases < 3 this would be a user input error
  }
  else
  {
    phs = phs.substr( dot + 1, phs.size() ) ;
    if ( pElem.Get_NPhases() == 1 )
    {
      if ( phs.find( "1.2" ) > 0 )
        result = "A";
      else
        if ( phs.find( "2.1" ) > 0 )
          result = "A";
        else
          if ( phs.find( "2.3" ) > 0 )
            result = "B";
          else
            if ( phs.find( "3.2" ) > 0 )
              result = "B";
            else
              if ( phs.find( "1.3" ) > 0 )
                result = "C";
              else
                if ( phs.find( "3.1" ) > 0 )
                  result = "C";
    }
    else
    {
      if ( phs.find( "1.2.3" ) > 0 )
        result = "AB";
      else
        if ( phs.find( "1.3.2" ) > 0 )
          result = "CB";
        else
          if ( phs.find( "2.1.3" ) > 0 )
            result = "AC";
          else
            if ( phs.find( "2.3.1" ) > 0 )
              result = "BC";
            else
              if ( phs.find( "3.1.2" ) > 0 )
                result = "CA";
              else
                if ( phs.find( "3.2.1" ) > 0 )
                  result = "BA";
    }
  }
  return result;
}
//#pragma resource "+"

TBankObject::TBankObject( int MaxWdg )
 : maxWindings(MaxWdg),
   nWindings(0),
   bAuto(false)
{
    /*
  connections.Length = MaxWdg;
  angles.Length = MaxWdg;
  phaseA.Length = MaxWdg;
  phaseB.Length = MaxWdg;
  phaseC.Length = MaxWdg;
  ground.Length = MaxWdg;
  */
  inherited( "Bank" );

}


TBankObject::~TBankObject( )
{
  free(connections);
  free(angles);
  free(phaseA);
  free(phaseB);
  free(phaseC);
  free(ground);
  pd_unit = NULL;
  // todo check:  inherited::Destroy;
}


void TBankObject::BuildVectorGroup( )
{

/*
  int i = 0;
  if ( bAuto )
  {
    if ( nWindings < 3 )
      vectorGroup = "YNa";
    else
      vectorGroup = "YNad1";
    return;
  }
  vectorGroup = "";
  i = 0; // dynamic arrays are zero-based
  while ( i < nWindings )
  {
    if ( ( phaseA[i] > 0 ) && ( phaseB[i] > 0 ) && ( phaseC[i] > 0 ) )
    {
      if ( connections[i] > 0 )
        vectorGroup = vectorGroup + "d";
      else
        vectorGroup = vectorGroup + "y";
      if ( ground[i] > 0 )
        vectorGroup = vectorGroup + "n";
      if ( angles[i] > 0 )
        vectorGroup = vectorGroup + IntToStr( angles[i] );
    }
    else
      vectorGroup = vectorGroup + "i";
    i++;
  }
  if ( vectorGroup.size()  > 0 )
    vectorGroup = UpperCase( LeftStr( vectorGroup, 1 ) ) + RightStr( vectorGroup, vectorGroup.size() ) - 1 );

*/
}


void TBankObject::AddTransformer( TTransfObj pXf )
{/*
  int i = 0;
  String phs;
  if ( pXf.NumberOfWindings > nWindings )
    nWindings = pXf.NumberOfWindings;
  pd_unit = pXf;
  for ( int stop = pXf.NumberOfWindings, i = 1; i <= stop; i++)
  {
    phs = PhaseString( pXf, i );
    if ( phs.find( "A" ) > 0 )
      phaseA[i - 1] = 1;
    if ( phs.find( "B" ) > 0 )
      phaseB[i - 1] = 1;
    if ( phs.find( "C" ) > 0 )
      phaseC[i - 1] = 1;
    connections[i - 1] = pXf.WdgConnection[i];
    if ( connections[i - 1] != connections[0] )
      angles[i - 1] = 1;
    if ( ( pXf.WdgRneutral[i] >= 0.0 ) || ( pXf.WdgXneutral[i] > 0.0 ) )
      if ( connections[i - 1] < 1 )
        ground[i - 1] = 1;
  }
  */
}


void TBankObject::AddAutoTransformer( TAutoTransObj pAuto ) // 3-phase, 2 or 3 windings

{
    /*
  int i = 0;
  if ( pAuto.NumberOfWindings > nWindings )
    nWindings = pAuto.NumberOfWindings;
  bAuto = true;
  pd_unit = pAuto;
  for ( int stop = pAuto.NumberOfWindings, i = 1; i <= stop; i++)
  {
    phaseA[i - 1] = 1;
    phaseB[i - 1] = 1;
    phaseC[i - 1] = 1;
    connections[i - 1] = pAuto.WdgConnection[i];
    if ( i == 2 )
      ground[i - 1] = 1;
  }
  */
}

TOpLimitObject::TOpLimitObject( double norm, double emerg )
 : NormAmps(norm),
   EmergAmps(emerg)
{
  inherited( "OpLimI" );
}


TOpLimitObject::~TOpLimitObject( )
{
  // todo check:  inherited::Destroy;
}

// the CIM transformer model requires some identified objects that don't have
// a counterpart in the DSS named objects.  These include banks, windings, and
// winding info.  So we create temporary UUIDs on the fly, and use a hash list when we
// need the UUIDs for later reference


void StartUuidList( int Size )
{
    /*
  if (( UuidList != NULL ) )
    FreeUuidList();
  UuidHash = new THashList( Size );
  UuidList.Length = Size;
  UuidKeyList.Length = Size;
  */
}


void StartBankList( int Size )
{
    /*
  BankHash = THashList.Create( Size );
  BankList.Length = Size;
  */
}


void StartOpLimitList( int Size )
{/*
  OpLimitHash = THashList.Create( Size );
  OpLimitList.Length = Size;
  */
}


void FreeUuidList( )
{
    /*
  UuidHash.Clear();
  UuidList = TUuid*;
  UuidKeyList = String*;
  */
}


void FreeBankList( )
{
    /*
  BankHash.Free;
  BankList = TBankObject**();
  */
}


void FreeOpLimitList( )
{
    /*
  OpLimitHash.Free;
  OpLimitList = TOpLimitObject**();
  */
}


void AddBank( TBankObject* pBank )
{
    /*
  int ref = 0, Size = 0;
  ref = BankHash.Add( pBank->LocalName );
  Size = BankList.High + 1;
  if ( ref > Size )
    BankList.Length = 2 * Size;
  BankList[ref - 1] = pBank;
  */
}


TBankObject* GetBank( String sBank )
{
  TBankObject* result = NULL;
  int ref = 0;
  result = NULL;
  ref = BankHash.Find( sBank );
  if ( ref > 0 )
    result = BankList[ref - 1];
  return result;
}


void AddOpLimit( TOpLimitObject* pLimit )
{
    /*
  int ref = 0, Size = 0;
  ref = OpLimitHash.Add( pLimit->LocalName );
  Size = OpLimitList.High + 1;
  if ( ref > Size )
    OpLimitList.Length = 2 * Size;
  OpLimitList[ref - 1] = pLimit;
  */
}


TOpLimitObject* GetOpLimit( String sLimit )
{
    
  TOpLimitObject* result = NULL;/*
  int ref = 0;
  result = NULL;
  ref = OpLimitHash.Find( sLimit );
  if ( ref > 0 )
    result = OpLimitList[ref - 1];*/
  return result;
  
}


TUuid GetHashedUuid( String key )
{
  TUuid result = {0};
  int ref = 0;
  int Size = 0;
  /*
  ref = UuidHash->Find(key);
  if ( ref == 0 )
  {
    ref = UuidHash.Add( key );
    CreateUUID4( result );  // this should be the ONLY place to call CreateUUID4
    Size = UuidList.High + 1;
    if ( ref > Size )
    {
      UuidList.Length = 2 * ( Size + 1 );
      UuidKeyList.Length = 2 * ( Size + 1 );
    }
    UuidList[ref - 1] = result;
    UuidKeyList[ref - 1] = key;
    
  }
  else
  {
    result = UuidList[ref - 1];
  }
  */
  return result;
}


void AddHashedUUID( String key, String UuidVal )
{
  int ref = 0;
  int Size = 0;
  /*
  ref = UuidHash->Find( key );
  if ( ref == 0 )
  {
    ref = UuidHash.Add( key );
    Size = UuidList.High + 1;
    if ( ref > Size )
    {
      UuidList.Length = 2 * ( Size + 1 );
      UuidKeyList.Length = 2 * ( Size + 1 );
    }
    UuidList[ref - 1] = StringToUuid( UuidVal );
    UuidKeyList[ref - 1] = key;
    
  }
  else
  {
   // UuidList[ref - 1] = StringToUuid( UuidVal );
  }
  */
}

// any temporary object (not managed by DSS) should have '=' prepended to the Name



TUuid GetDevUuid( UuidChoice which, String Name, int Seq )
{
  TUuid result;
  String key;
  switch ( which )
  {
    case Bank:
      key = "Bank=";
    break;
    case Wdg:
      key = "Wdg=";
    break;
    case XfCore:
      key = "XfCore=";
    break;
    case XfMesh:
      key = "XfMesh=";
    break;
    case WdgInf:
      key = "WdgInf=";
    break;
    case ScTest:
      key = "ScTest=";
    break;
    case OcTest:
      key = "OcTest=";
    break;
    case BaseV:
      key = "BaseV=";
    break;
    case OpLimV:
      key = "OpLimV=";
    break;
    case OpLimI:
      key = "OpLimI=";
    break;
    case LinePhase:
      key = "LinePhase=";
    break;
    case LoadPhase:
      key = "LoadPhase=";
    break;
    case GenPhase:
      key = "GenPhase=";
    break;
    case SolarPhase:
      key = "PVPhase=";
    break;
    case BatteryPhase:
      key = "BattPhase=";
    break;
    case CapPhase:
      key = "CapPhase=";
    break;
    case XfLoc:
      key = "XfLoc=";
    break;
    case LoadLoc:
      key = "LoadLoc=";
    break;
    case LineLoc:
      key = "LineLoc=";
    break;
    case ReacLoc:
      key = "ReacLoc=";
    break;
    case CapLoc:
      key = "CapLoc=";
    break;
    case Topo:
      key = "Topo=";
    break;
    case SolarLoc:
      key = "SolarLoc=";
    break;
    case BatteryLoc:
      key = "BatteryLoc=";
    break;
    case LoadResp:
      key = "LoadResp=";
    break;
    case CIMVer:
      key = "CIMVer=";
    break;
    case ZData:
      key = "ZData=";
    break;
    case PosPt:
      key = "PosPt=";
    break;
    case CoordSys:
      key = "CoordSys=";
    break;
    case TopoIsland:
      key = "TopoIsland=";
    break;
    case OpLimT:
      key = "OpLimT=";
    break;
    case Station:
      key = "Station=";
    break;
    case GeoRgn:
      key = "GeoRgn=";
    break;
    case SubGeoRgn:
      key = "SubGeoRgn=";
    break;
    case FdrLoc:
      key = "FdrLoc=";
    break;
    case XfInfo:
      key = "XfInfo=";
    break;
    case OpLimAHi:
      key = "OpLimAHi=";
    break;
    case OpLimALo:
      key = "OpLimALo=";
    break;
    case OpLimBHi:
      key = "OpLimBHi=";
    break;
    case OpLimBLo:
      key = "OpLimBLo=";
    break;
    case MachLoc:
      key = "MachLoc=";
    break;
    case SrcLoc:
      key = "SrcLoc=";
    break;
    case PVPanels:
      key = "PVPanels=";
    break;
    case Battery:
      key = "Battery=";
    break;
    case TankInfo:
      key = "TankInfo=";
    break;
    case TankAsset:
      key = "TankAsset=";
    break;
    case TapInfo:
      key = "TapInfo=";
    break;
    case TapCtrl:
      key = "TapCtrl=";
    break;
    case TapAsset:
      key = "TapAsset=";
    break;
    case PUZ:
      key = "PUZ=";
    break;
    case WirePos:
      key = "WirePos=";
    break;
    case NormAmps:
      key = "NormAmps=";
    break;
    case EmergAmps:
      key = "EmergAmps=";
    break;
    case I1547NameplateData:
      key = "INameplate=";
    break;
    case I1547NameplateDataApplied:
      key = "IApplied=";
    break;
    case I1547Signal:
      key = "ISignal=";
    break;
    case I1547VoltVar:
      key = "IVVar=";
    break;
    case I1547WattVar:
      key = "IWVar=";
    break;
    case I1547ConstPF:
      key = "IPF=";
    break;
    case I1547VoltWatt:
      key = "IVWatt=";
    break;
    case I1547ConstQ:
      key = "IQ=";
    break;
  }
  key = key + Name + "=" + IntToStr( Seq );
  result = GetHashedUuid( key );
  return result;
}


void DefaultCircuitUUIDs( TUuid& fdrID, TUuid& subID, TUuid& rgnID, TUuid& subGeoID )
{
  if ( !( UuidList != NULL ) )
    StartUuidList( ActiveCircuit[ActiveActor]->NumBuses + 2 * ActiveCircuit[ActiveActor]->NumDevices );
  //fdrID = ActiveCircuit[ActiveActor]->uuid;
  subID = GetDevUuid( Station, "Station", 1 );
  rgnID = GetDevUuid( GeoRgn, "GeoRgn", 1 );
  subGeoID = GetDevUuid( SubGeoRgn, "SubGeoRgn", 1 );
}


void WriteHashedUUIDs( Textfile& F )
{
    /*
  int i = 0;
  for ( int stop = UuidList.High, i = 0; i <= stop; i++)
  {
    if ( UuidKeyList[i].size() ) < 1 )
      Break;
     WriteLn( Format( "%s %s", VECTOROFCONST(( UuidKeyList[i], UUIDToString( UuidList[i] ) )) ) );
  }
  */
}

// terminals are uniquely identified by class (DSSObjType), plus name and sequence



TUuid GetTermUuid( TDSSCktElement pElem, int Seq )
{
  TUuid result;
  String key;
  key = IntToStr( pElem.DSSObjType ) + "=" + pElem.get_Name() + "=" + IntToStr( Seq );
  result = GetHashedUuid( key );
  return result;
}
//#pragma resource "-"

String GetBaseVName( double val )
{
  String result;
//  Result := Format('BaseV_%.3f', [val]);
  result = "BaseV_";// + floattostrF( val, ffFixed, 6, 4 );
  return result;
}

TUuid GetBaseVUuid( double val )
{
  TUuid result;
  result = GetDevUuid( BaseV, GetBaseVName( val ), 1 );
  return result;
}


String GetOpLimVName( double val )
{
  String result;
  result = "OpLimV_";// +floattostrF(val, ffFixed, 6, 4);
  return result;
}


TUuid GetOpLimVUuid( double val )
{
  TUuid result;
  result = GetDevUuid( OpLimV, GetOpLimVName( val ), 1 );
  return result;
}


String GetOpLimIName( double norm, double emerg )
{
  String result;
  result = "OpLimI_";// +floattostrF(norm, ffFixed, 6, 1) + "_" + floattostrF(emerg, ffFixed, 6, 1);
  return result;
}


TUuid GetOpLimIUuid( double norm, double emerg )
{
  TUuid result;
  result = GetDevUuid( OpLimI, GetOpLimIName( norm, emerg ), 1 );
  return result;
}


void DoubleNode( ProfileChoice prf, String Node, double val )
{
  //FD->WriteCimLn( prf, Format( "  <cim:%s>%.8g</cim:%s>", VECTOROFCONST(( Node, val, Node )) ) );
}


void IntegerNode( ProfileChoice prf, String Node, int val )
{
  //FD->WriteCimLn( prf, Format( "  <cim:%s>%d</cim:%s>", VECTOROFCONST(( Node, val, Node )) ) );
}


void BooleanNode( ProfileChoice prf, String Node, bool val )
{
  String i;
  if ( val )
    i = "true";
  else
    i = "false";
//  FD->WriteCimLn( prf, Format( "  <cim:%s>%s</cim:%s>", VECTOROFCONST(( Node, i, Node )) ) );
}


void RefNode( ProfileChoice prf, String Node, TNamedObject Obj )
{
  //FD->WriteCimLn( prf, Format( "  <cim:%s rdf:resource=\"#%s\"/>", VECTOROFCONST(( Node, Obj.CIM_ID )) ) );
}


void UuidNode( ProfileChoice prf, String Node, TUuid ID )
{
  //FD->WriteCimLn( prf, Format( "  <cim:%s rdf:resource=\"#%s\"/>", VECTOROFCONST(( Node, UUIDToCIMString( ID ) )) ) );
}


void LineCodeRefNode( ProfileChoice prf, TLineCode List, String Name )
{
  TLineCodeObj* Obj;
  if ( List.SetActive( Name ) )
  {
    Obj = (TLineCodeObj*) List.GetActiveObj();
//    FD->WriteCimLn( prf, Format( "  <cim:ACLineSegment.PerLengthImpedance rdf:resource=\"#%s\"/>", VECTOROFCONST(( Obj.CIM_ID )) ) );
  }
}


void LineSpacingRefNode( ProfileChoice prf, TDSSClass List, String Name )
{
  TDSSObject* Obj; // should be a TLineGeometryObj or TLineSpacingObj
  if ( List.SetActive( Name ) )
  {
    Obj = (TLineCodeObj *)  List.GetActiveObj();
//    FD->WriteCimLn( prf, Format( "  <cim:ACLineSegment.WireSpacingInfo rdf:resource=\"#%s\"/>", VECTOROFCONST(( Obj.CIM_ID )) ) );
  }
}

void PhaseWireRefNode( ProfileChoice prf, TConductorDataObj Obj )
{
//  FD->WriteCimLn( prf, Format( "  <cim:ACLineSegmentPhase.WireInfo rdf:resource=\"#%s\"/>", VECTOROFCONST(( Obj.CIM_ID )) ) );
}


void CircuitNode( ProfileChoice prf, TNamedObject Obj )
{
//  FD->WriteCimLn( prf, Format( "  <cim:Equipment.EquipmentContainer rdf:resource=\"#%s\"/>", VECTOROFCONST(( Obj.CIM_ID )) ) );
}


String FirstPhaseString( TDSSCktElement pElem, int Bus )
{
  String result;
  String val;
  val = PhaseString( pElem, Bus );
  /*
  if ( !val.IsEmpty())
    result = LeftStr( val, 1 );
  else*/
    result = "A";
  return result;
}


void GeneratorControlEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:GeneratingUnit.genControlSource rdf:resource=\"%s#GeneratorControlSource.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void BatteryStateEnum( ProfileChoice prf, int val )
{
  String str;
  str = "Waiting";
  if ( val == STORE_CHARGING )
    str = "Charging";
  else
    if ( val == STORE_DISCHARGING )
      str = "Discharging";
//  FD->WriteCimLn( prf, Format( "  <cim:BatteryUnit.batteryState rdf:resource=\"%s#BatteryState.%s\"/>", VECTOROFCONST(( CIM_NS, str )) ) );
}


void SynchMachTypeEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:SynchronousMachine.type rdf:resource=\"%s#SynchronousMachineType.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void SynchMachModeEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:SynchronousMachine.operatingMode rdf:resource=\"%s#SynchronousMachineOperatingMode.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void RegulatingControlEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:RegulatingControl.mode rdf:resource=\"%s#RegulatingControlModeKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void WindingConnectionEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:TransformerEndInfo.connectionKind rdf:resource=\"%s#WindingConnection.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void ConductorInsulationEnum( ProfileChoice prf, String val )
{
 // FD->WriteCimLn( prf, Format( "  <cim:WireInfo.insulationMaterial rdf:resource=\"%s#WireInsulationKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void ConductorUsageEnum( ProfileChoice prf, String val )
{
 // FD->WriteCimLn( prf, Format( "  <cim:WireSpacingInfo.usage rdf:resource=\"%s#WireUsageKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void CableShieldMaterialEnum( ProfileChoice prf, String val )
{
//  FD.WriteCimLn (prf, Format ('  <cim:CableInfo.shieldMaterial rdf:resource="%s#CableShieldMaterialKind.%s"/>',
//    [CIM_NS, val]));
}


void ConductorMaterialEnum( ProfileChoice prf, String val )
{
//  FD.WriteCimLn (prf, Format ('  <cim:WireInfo.material rdf:resource="%s#WireMaterialKind.%s"/>',
//    [CIM_NS, val]));
}


void CableOuterJacketEnum( ProfileChoice prf, String val )
{
//  FD.WriteCimLn (prf, Format ('  <cim:CableInfo.outerJacketKind rdf:resource="%s#CableOuterJacketKind.%s"/>',
//    [CIM_NS, val]));
}


void CableConstructionEnum( ProfileChoice prf, String val )
{
//  FD.WriteCimLn (prf, Format ('  <cim:CableInfo.constructionKind rdf:resource="%s#CableConstructionKind.%s"/>',
//    [CIM_NS, val]));
}


void TransformerControlEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:RatioTapChanger.tculControlMode rdf:resource=\"%s#TransformerControlMode.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void MonitoredPhaseNode( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:RegulatingControl.monitoredPhase rdf:resource=\"%s#PhaseCode.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void OpLimitDirectionEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:OperationalLimitType.direction rdf:resource=\"%s#OperationalLimitDirectionKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}

// next several for DERIEEEType1 CIM dynamics



void NormalOpCatEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:DERNameplateData.normalOPcatKind rdf:resource=\"%s#NormalOPcatKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void SupportedModesEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:DERNameplateData.supportedModesKind rdf:resource=\"%s#SupportedModesKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void PowerFactorExcitationEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:ConstantPowerFactorSettings.constantPFexcitationKind rdf:resource=\"%s#ConstantPowerFactorSettingKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}


void RemoteInputSignalEnum( ProfileChoice prf, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:RemoteInputSignal.remoteSignalType rdf:resource=\"%s#RemoteSignalKind.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}
// end of Enums for DERIEEEType1 CIM dynamics



void StringNode( ProfileChoice prf, String Node, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:%s>%s</cim:%s>", VECTOROFCONST(( Node, val, Node )) ) );
}


void StartInstance( ProfileChoice prf, String Root, TNamedObject Obj )
{
  FD->StartInstance( prf, Root, Obj );
}


void StartFreeInstance( ProfileChoice prf, String Root, TUuid uuid )
{
  FD->StartFreeInstance( prf, Root, uuid );
}


void EndInstance( ProfileChoice prf, String Root )
{
  FD->EndInstance( prf, Root );
}

void XfmrTankPhasesAndGround( ProfileChoice fprf, ProfileChoice eprf, TTransfObj pXf, int Bus )
{
  String ordered_phs, phs;
  bool reversed = false;
  int j1 = 0
//  j, jmax: Integer;
  , j2 = 0;
//  writeln(Format ('Xfmr Tank: %s end: %d Nconds: %d Nterms: %d Nphases: %d', [pXf.LocalName, bus, pXf.Nconds, pXf.Nterms, pXf.Nphases]));
  reversed = false;
  ordered_phs = PhaseOrderString( pXf, Bus );
  if ( ( ordered_phs == "BCA" ) || ( ordered_phs == "CAB" ) )
  {  // 'ABC' already fine
    phs = "ABC";
  }
  else
    if ( ( ordered_phs == "ACB" ) || ( ordered_phs == "BAC" ) || ( ordered_phs == "CBA" ) )
    {
      phs = "ABC";
      reversed = true;
    }
    else
      if ( ordered_phs == "BA" )
      {
        phs = "AB";
        reversed = true;
      }
      else
        if ( ordered_phs == "CA" )
        {
          phs = "AC";
          reversed = true;
        }
        else
          if ( ordered_phs == "CB" )
          {
            phs = "BC";
            reversed = true;
          }
          else
            if ( ordered_phs == "s2" )
            {
              phs = "s2";
              reversed = true;
            }
            else
            {
              phs = ordered_phs;
            }
//  FD->WriteCimLn( fprf, Format( "  <cim:TransformerTankEnd.phases rdf:resource=\"%s#PhaseCode.%s\"/>", VECTOROFCONST(( CIM_NS, phs )) ) );
  // interpret the grounding and reversal connections
//  jmax := pXf.NConds * pXf.NTerms;
//  for j := 1 to jmax do begin
//    writeln(Format ('  j: %d, noderef^[j]: %d', [j, pXf.NodeRef^[j]]));
//  end;
  j1 = ( Bus - 1 ) * pXf.Get_NConds() + 1;
  j2 = j1 + pXf.Get_NPhases();
//  writeln(Format('  Testing %d and %d', [j1, j2]));
/*  if (pXf.Winding[Bus].Connection == 1)
  { // delta
    BooleanNode( fprf, "TransformerEnd.grounded", false );
  }
  else
    if ( pXf.NodeRef[j2] == 0 )
    { // last conductor is grounded solidly
      BooleanNode( FunPrf, "TransformerEnd.grounded", true );
      DoubleNode( EpPrf, "TransformerEnd.rground", 0.0 );
      DoubleNode( EpPrf, "TransformerEnd.xground", 0.0 );
    }
    else
      if ( pXf.NodeRef[j1] == 0 )
      { // first conductor is grounded solidly, but should be reversed
        BooleanNode( FunPrf, "TransformerEnd.grounded", true );
        DoubleNode( EpPrf, "TransformerEnd.rground", 0.0 );
        DoubleNode( EpPrf, "TransformerEnd.xground", 0.0 );
        reversed = true;
      }
      else
        if ( pXf.Winding[Bus].Rneut < 0.0 )
        { // probably wye ungrounded
          BooleanNode( FunPrf, "TransformerEnd.grounded", false );
        }
        else
        { // not delta, not wye solidly grounded or ungrounded
          BooleanNode( FunPrf, "TransformerEnd.grounded", true );
          DoubleNode( EpPrf, "TransformerEnd.rground", pXf.Winding[Bus].Rneut );
          DoubleNode( EpPrf, "TransformerEnd.xground", pXf.Winding[Bus].Xneut );
        }*/
  BooleanNode( fprf, "TransformerTankEnd.reversed", reversed );
}


void PhaseNode( ProfileChoice prf, String Root, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:%s.phase rdf:resource=\"%s#PhaseCode.%s\"/>", VECTOROFCONST(( Root, CIM_NS, val )) ) );
}


void PhaseKindNode( ProfileChoice prf, String Root, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:%s.phase rdf:resource=\"%s#SinglePhaseKind.%s\"/>", VECTOROFCONST(( Root, CIM_NS, val )) ) );
}


void PhaseSideNode( ProfileChoice prf, String Root, int Side, String val )
{
//  FD->WriteCimLn( prf, Format( "  <cim:%s.phaseSide%d rdf:resource=\"%s#SinglePhaseKind.%s\"/>", VECTOROFCONST(( Root, Side, CIM_NS, val )) ) );
}


void ShuntConnectionKindNode( ProfileChoice prf, String Root, String val ) // D, Y, Yn, I

{
//  FD->WriteCimLn( prf, Format( "  <cim:%s.phaseConnection rdf:resource=\"%s#PhaseShuntConnectionKind.%s\"/>", VECTOROFCONST(( Root, CIM_NS, val )) ) );
}


void WindingConnectionKindNode( ProfileChoice prf, String val ) // D, Y, Z, Yn, Zn, A, I

{
//  FD->WriteCimLn( prf, Format( "  <cim:PowerTransformerEnd.connectionKind rdf:resource=\"%s#WindingConnection.%s\"/>", VECTOROFCONST(( CIM_NS, val )) ) );
}

// we specify phases except for balanced three-phase



void AttachLinePhases( TLineObj pLine )
{
  String S, phs;
  int i = 0;/*
  TNamedObject pPhase;
  pPhase = TNamedObject.Create( "dummy" );
  S = PhaseOrderString( pLine, 1 );
  if ( pLine.NumConductorsAvailable > S.size() ) )
    S = S + "N"; // so we can specify the neutral conductor
  for ( int stop = S.size() ), i = 1; i <= stop; i++)
  {
    phs = S[i];
    if ( phs == "s" )
      Continue;
    if ( phs == "1" )
      phs = "s1";
    if ( phs == "2" )
      phs = "s2";
    pPhase.LocalName = pLine.Name + "_" + phs;
    pPhase.uuid = GetDevUuid( LinePhase, pPhase.LocalName, 1 );
    StartInstance( FunPrf, "ACLineSegmentPhase", pPhase );
    PhaseKindNode( FunPrf, "ACLineSegmentPhase", phs );
    IntegerNode( FunPrf, "ACLineSegmentPhase.sequenceNumber", i );
    if ( i <= pLine.NumConductorsAvailable )
      PhaseWireRefNode( CatPrf, pLine.ConductorData[i] );
    RefNode( FunPrf, "ACLineSegmentPhase.ACLineSegment", pLine );
    UuidNode( GeoPrf, "PowerSystemResource.Location", GetDevUuid( LineLoc, pLine.Name, 1 ) );
    EndInstance( FunPrf, "ACLineSegmentPhase" );
  }*/
}


void AttachSwitchPhases( TLineObj pLine )
{
  String s1, s2, phs1, phs2;
  int i = 0;
  TNamedObject pPhase;/*
  // also write the switch phases if needed to support transpositions
  s1 = PhaseOrderString( pLine, 1 );
  s2 = PhaseOrderString( pLine, 2 );
  if ( ( pLine.NPhases == 3 ) && ( s1.size() ) == 3 ) && ( s1 == s2 ) )
    return;
  pPhase = TNamedObject.Create( "dummy" );
  for ( int stop = s1.size() ), i = 1; i <= stop; i++)
  {
    phs1 = s1[i];
    phs2 = s2[i];
    if ( phs1 == "s" )
      Continue;
    if ( phs2 == "s" )
      Continue;
    if ( phs1 == "1" )
      phs1 = "s1";
    if ( phs1 == "2" )
      phs1 = "s2";
    if ( phs2 == "1" )
      phs2 = "s1";
    if ( phs2 == "2" )
      phs2 = "s2";
    pPhase.LocalName = pLine.Name + "_" + phs1;
    pPhase.uuid = GetDevUuid( LinePhase, pPhase.LocalName, 1 );
    StartInstance( FunPrf, "SwitchPhase", pPhase );
    BooleanNode( SshPrf, "SwitchPhase.closed", pLine.Closed[0][ ActiveActor] );
    BooleanNode( FunPrf, "SwitchPhase.normalOpen", ~ pLine.Closed[0][ ActiveActor] );
    PhaseSideNode( FunPrf, "SwitchPhase", 1, phs1 );
    PhaseSideNode( FunPrf, "SwitchPhase", 2, phs2 );
    RefNode( FunPrf, "SwitchPhase.Switch", pLine );
    UuidNode( GeoPrf, "PowerSystemResource.Location", GetDevUuid( LineLoc, pLine.Name, 1 ) );
    EndInstance( FunPrf, "SwitchPhase" );
  }*/
}


void AttachCapPhases( TCapacitorObj pCap, TUuid geoUUID, double sections )
{
  String S, phs;
  int i = 0;
  TNamedObject pPhase;
  /**
  double bph = 0.0;
  if ( pCap.NPhases == 3 )
    return;
  pPhase = TNamedObject.Create( "dummy" );
  S = PhaseString( pCap, 1 );
  with pCap do
  {
    bph = double( double( double( double( 0.001 ) ) ) ) * Totalkvar / NomKV / NomKV / NumSteps / NPhases;
    if ( Connection == 1 )
      S = DeltaPhaseString( pCap );
  }
  for ( int stop = S.size() ), i = 1; i <= stop; i++)
  {
    phs = S[i];
    pPhase.LocalName = pCap.Name + "_" + phs;
    pPhase.uuid = GetDevUuid( CapPhase, pPhase.LocalName, 1 );
    StartInstance( FunPrf, "LinearShuntCompensatorPhase", pPhase );
    PhaseKindNode( FunPrf, "ShuntCompensatorPhase", phs );
    DoubleNode( EpPrf, "LinearShuntCompensatorPhase.bPerSection", bph );
    DoubleNode( EpPrf, "LinearShuntCompensatorPhase.gPerSection", 0.0 );
    IntegerNode( EpPrf, "ShuntCompensatorPhase.normalSections", pCap.NumSteps );
    IntegerNode( EpPrf, "ShuntCompensatorPhase.maximumSections", pCap.NumSteps );
    DoubleNode( SshPrf, "ShuntCompensatorPhase.sections", sections );
    RefNode( FunPrf, "ShuntCompensatorPhase.ShuntCompensator", pCap );
    UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
    EndInstance( FunPrf, "LinearShuntCompensatorPhase" );
  }
  */
}


void AttachSecondaryPhases( TLoadObj pLoad, TUuid geoUUID, TNamedObject pPhase, double p, double q, String phs )
{
    /**
  pPhase.LocalName = pLoad.Name + "_" + phs;
  pPhase.uuid = GetDevUuid( LoadPhase, pPhase.LocalName, 1 );
  StartInstance( FunPrf, "EnergyConsumerPhase", pPhase );
  PhaseKindNode( FunPrf, "EnergyConsumerPhase", phs );
  DoubleNode( SshPrf, "EnergyConsumerPhase.p", p );
  DoubleNode( SshPrf, "EnergyConsumerPhase.q", q );
  RefNode( FunPrf, "EnergyConsumerPhase.EnergyConsumer", pLoad );
  UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
  EndInstance( FunPrf, "EnergyConsumerPhase" );*/
}

void AttachLoadPhases( TLoadObj pLoad, TUuid geoUUID )
{
  String S, phs;
  int i = 0;
  TNamedObject pPhase;/**
  double p = 0.0, q = 0.0;
  bool bAllowSec = false;
  if ( pLoad.NPhases == 3 )
    return;
  // TODO - use a more robust filter than pLoad.LoadClass, which is > 1 only for PNNL taxonomy imports
  if ( pLoad.LoadClass <= 1 )
    bAllowSec = true;
  else
    bAllowSec = false;
  p = double( 1000.0 ) * pLoad.kWBase / pLoad.NPhases;
  q = double( 1000.0 ) * pLoad.kvarBase / pLoad.NPhases;
  if ( pLoad.Connection == 1 )
    S = DeltaPhaseString( pLoad );
  else
    S = PhaseString( pLoad, 1, bAllowSec );
  pPhase = TNamedObject.Create( "dummy" );
  // first, filter out what appear to be split secondary loads
  // these can be 2-phase loads (balanced) nominally 0.208 kV, or
  //  1-phase loads (possibly unbalanced) nominally 0.12 kV
  //  TODO - handle s1 to s2 240-volt loads; these would be s12, which is not a valid SinglePhaseKind
  if ( ( pLoad.kVLoadBase < 0.25 ) && bAllowSec )
  {
    if ( pLoad.NPhases == 2 )
    {
      AttachSecondaryPhases( pLoad, geoUUID, pPhase, p, q, "s1" );
      AttachSecondaryPhases( pLoad, geoUUID, pPhase, p, q, "s2" );
      return;
    }
    else
    {
      AttachSecondaryPhases( pLoad, geoUUID, pPhase, p, q, S );
      return;
    }
  }
  for ( int stop = S.size() ), i = 1; i <= stop; i++)
  {
    phs = S[i];
    pPhase.LocalName = pLoad.Name + "_" + phs;
    pPhase.uuid = GetDevUuid( LoadPhase, pPhase.LocalName, 1 );
    StartInstance( FunPrf, "EnergyConsumerPhase", pPhase );
    PhaseKindNode( FunPrf, "EnergyConsumerPhase", phs );
    DoubleNode( SshPrf, "EnergyConsumerPhase.p", p );
    DoubleNode( SshPrf, "EnergyConsumerPhase.q", q );
    RefNode( FunPrf, "EnergyConsumerPhase.EnergyConsumer", pLoad );
    UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
    EndInstance( FunPrf, "EnergyConsumerPhase" );
  }*/
}


void AttachSecondaryGenPhases( TGeneratorObj pGen, TUuid geoUUID, TNamedObject pPhase, double p, double q, String phs )
{
    /*
  pPhase.LocalName = pGen.Name + "_" + phs;
  pPhase.uuid = GetDevUuid( GenPhase, pPhase.LocalName, 1 );
  StartInstance( FunPrf, "SynchronousMachinePhase", pPhase );
  PhaseKindNode( FunPrf, "SynchronousMachinePhase", phs );
  DoubleNode( SshPrf, "SynchronousMachinePhase.p", p );
  DoubleNode( SshPrf, "SynchronousMachinePhase.q", q );
  RefNode( FunPrf, "SynchronousMachinePhase.SynchronousMachine", pGen );
  UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
  EndInstance( FunPrf, "SynchronousMachinePhase" );*/
}


void AttachGeneratorPhases( TGeneratorObj pGen, TUuid geoUUID )
{
  String S, phs;
  int i = 0;
  TNamedObject pPhase;/*
  double p = 0.0, q = 0.0;
  if ( pGen.NPhases == 3 )
    return;
  p = double( 1000.0 ) * pGen.Presentkw / pGen.NPhases;
  q = double( 1000.0 ) * pGen.Presentkvar / pGen.NPhases;
  if ( pGen.Connection == 1 )
    S = DeltaPhaseString( pGen );
  else
    S = PhaseString( pGen, 1 );
  pPhase = TNamedObject.Create( "dummy" );
  //  TODO - handle s1 to s2 240-volt loads; these would be s12, which is not a valid SinglePhaseKind
  if ( pGen.Presentkv < 0.25 )
  {
    if ( pGen.NPhases == 2 )
    {
      AttachSecondaryGenPhases( pGen, geoUUID, pPhase, p, q, "s1" );
      AttachSecondaryGenPhases( pGen, geoUUID, pPhase, p, q, "s2" );
      return;
    }
    else
    {
      AttachSecondaryGenPhases( pGen, geoUUID, pPhase, p, q, S );
      return;
    }
  }
  for ( int stop = S.size() ), i = 1; i <= stop; i++)
  {
    phs = S[i];
    pPhase.LocalName = pGen.Name + "_" + phs;
    pPhase.uuid = GetDevUuid( GenPhase, pPhase.LocalName, 1 );
    StartInstance( FunPrf, "SynchronousMachinePhase", pPhase );
    PhaseKindNode( FunPrf, "SynchronousMachinePhase", phs );
    DoubleNode( SshPrf, "SynchronousMachinePhase.p", p );
    DoubleNode( SshPrf, "SynchronousMachinePhase.q", q );
    RefNode( FunPrf, "SynchronousMachinePhase.SynchronousMachine", pGen );
    UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
    EndInstance( FunPrf, "SynchronousMachinePhase" );
  }*/
}

void AttachSecondarySolarPhases( TPVsystemObj pPV, TUuid geoUUID, TNamedObject pPhase, double p, double q, String phs )
{
    /*
  pPhase.LocalName = pPV.Name + "_" + phs;
  pPhase.uuid = GetDevUuid( SolarPhase, pPhase.LocalName, 1 );
  StartInstance( FunPrf, "PowerElectronicsConnectionPhase", pPhase );
  PhaseKindNode( FunPrf, "PowerElectronicsConnectionPhase", phs );
  DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.p", p );
  DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.q", q );
  RefNode( FunPrf, "PowerElectronicsConnectionPhase.PowerElectronicsConnection", pPV );
  UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
  EndInstance( FunPrf, "PowerElectronicsConnectionPhase" );*/
}


void AttachSolarPhases( TPVsystemObj pPV, TUuid geoUUID )
{
  String S, phs;
  int i = 0;
  TNamedObject pPhase;/**
  double p = 0.0, q = 0.0;
  if ( pPV.NPhases == 3 )
    return;
  p = double( 1000.0 ) * pPV.Presentkw / pPV.NPhases;
  q = double( 1000.0 ) * pPV.Presentkvar / pPV.NPhases;
  if ( pPV.Connection == 1 )
    S = DeltaPhaseString( pPV );
  else
    S = PhaseString( pPV, 1 );
  pPhase = TNamedObject.Create( "dummy" );
  //  TODO - handle s1 to s2 240-volt loads; these would be s12, which is not a valid SinglePhaseKind
  if ( pPV.Presentkv < 0.25 )
  {
    if ( pPV.NPhases == 2 )
    {
      AttachSecondarySolarPhases( pPV, geoUUID, pPhase, p, q, "s1" );
      AttachSecondarySolarPhases( pPV, geoUUID, pPhase, p, q, "s2" );
      return;
    }
    else
    {
      AttachSecondarySolarPhases( pPV, geoUUID, pPhase, p, q, S );
      return;
    }
  }
  for ( int stop = S.size() ), i = 1; i <= stop; i++)
  {
    phs = S[i];
    pPhase.LocalName = pPV.Name + "_" + phs;
    pPhase.uuid = GetDevUuid( SolarPhase, pPhase.LocalName, 1 );
    StartInstance( FunPrf, "PowerElectronicsConnectionPhase", pPhase );
    PhaseKindNode( FunPrf, "PowerElectronicsConnectionPhase", phs );
    DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.p", p );
    DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.q", q );
    RefNode( FunPrf, "PowerElectronicsConnectionPhase.PowerElectronicsConnection", pPV );
    UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
    EndInstance( FunPrf, "PowerElectronicsConnectionPhase" );
  }*/
}


void AttachSecondaryStoragePhases( TStorageObj pBat, TUuid geoUUID, TNamedObject pPhase, double p, double q, String phs )
{/**
  pPhase.LocalName = pBat.Name + "_" + phs;
  pPhase.uuid = GetDevUuid( BatteryPhase, pPhase.LocalName, 1 );
  StartInstance( FunPrf, "PowerElectronicsConnectionPhase", pPhase );
  PhaseKindNode( FunPrf, "PowerElectronicsConnectionPhase", phs );
  DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.p", p );
  DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.q", q );
  RefNode( FunPrf, "PowerElectronicsConnectionPhase.PowerElectronicsConnection", pBat );
  UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
  EndInstance( FunPrf, "PowerElectronicsConnectionPhase" );*/
}


void AttachStoragePhases( TStorageObj pBat, TUuid geoUUID )
{
  String S, phs;
  int i = 0;
  TNamedObject pPhase;/**
  double p = 0.0, q = 0.0;
  if ( pBat.NPhases == 3 )
    return;
  p = double( 1000.0 ) * pBat.Presentkw / pBat.NPhases;
  q = double( 1000.0 ) * pBat.Presentkvar / pBat.NPhases;
  if ( pBat.Connection == 1 )
    S = DeltaPhaseString( pBat );
  else
    S = PhaseString( pBat, 1 );
  pPhase = TNamedObject.Create( "dummy" );
  //  TODO - handle s1 to s2 240-volt loads; these would be s12, which is not a valid SinglePhaseKind
  if ( pBat.Presentkv < 0.25 )
  {
    if ( pBat.NPhases == 2 )
    {
      AttachSecondaryStoragePhases( pBat, geoUUID, pPhase, p, q, "s1" );
      AttachSecondaryStoragePhases( pBat, geoUUID, pPhase, p, q, "s2" );
      return;
    }
    else
    {
      AttachSecondaryStoragePhases( pBat, geoUUID, pPhase, p, q, S );
      return;
    }
  }
  for ( int stop = S.size() ), i = 1; i <= stop; i++)
  {
    phs = S[i];
    pPhase.LocalName = pBat.Name + "_" + phs;
    pPhase.uuid = GetDevUuid( BatteryPhase, pPhase.LocalName, 1 );
    StartInstance( FunPrf, "PowerElectronicsConnectionPhase", pPhase );
    PhaseKindNode( FunPrf, "PowerElectronicsConnectionPhase", phs );
    DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.p", p );
    DoubleNode( SshPrf, "PowerElectronicsConnectionPhase.q", q );
    RefNode( FunPrf, "PowerElectronicsConnectionPhase.PowerElectronicsConnection", pBat );
    UuidNode( GeoPrf, "PowerSystemResource.Location", geoUUID );
    EndInstance( FunPrf, "PowerElectronicsConnectionPhase" );
  }*/
}

void WriteLoadModel( String Name, TUuid ID, double zP, double iP, double pP, double zQ, double iQ, double pQ, double eP, double eQ )
{/**
  FD->WriteCimLn( FunPrf, Format( "<cim:LoadResponseCharacteristic rdf:ID=\"%s\">", VECTOROFCONST(( UUIDToCIMString( ID ) )) ) );
  StringNode( FunPrf, "IdentifiedObject.mRID", UUIDToCIMString( ID ) );
  StringNode( FunPrf, "IdentifiedObject.name", Name );
  if ( ( eP > 0.0 ) || ( eQ > 0.0 ) )
    BooleanNode( FunPrf, "LoadResponseCharacteristic.exponentModel", true );
  else
    BooleanNode( FunPrf, "LoadResponseCharacteristic.exponentModel", false );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.pConstantImpedance", zP );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.pConstantCurrent", iP );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.pConstantPower", pP );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.qConstantImpedance", zQ );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.qConstantCurrent", iQ );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.qConstantPower", pQ );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.pVoltageExponent", eP );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.qVoltageExponent", eQ );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.pFrequencyExponent", 0.0 );
  DoubleNode( FunPrf, "LoadResponseCharacteristic.qFrequencyExponent", 0.0 );
  FD->WriteCimLn( FunPrf, "</cim:LoadResponseCharacteristic>" );*/
}


bool IsGroundBus( const String S )
{
  bool result = false;
  int i = 0;
  result = true;
  i = S.find( ".1" );
  if ( i > 0 )
    result = false;
  i = S.find( ".2" );
  if ( i > 0 )
    result = false;
  i = S.find( ".3" );
  if ( i > 0 )
    result = false;
  i = S.find( "." );
  if ( i == 0 )
    result = false;
  return result;
}


void WritePositions( TDSSCktElement pElem, TUuid geoUUID, TUuid crsUUID )
{
  int Nterm = 0, j = 0, ref = 0;
  String BusName;/**
  Nterm = pElem.Nterms;
  BusName = pElem.FirstBus;
  StartFreeInstance( GeoPrf, "Location", geoUUID );
  StringNode( GeoPrf, "IdentifiedObject.mRID", UUIDToCIMString( geoUUID ) );
  StringNode( GeoPrf, "IdentifiedObject.name", pElem.LocalName + "_Loc" );
  UuidNode( GeoPrf, "Location.CoordinateSystem", crsUUID );
  EndInstance( GeoPrf, "Location" );
  for ( int stop = Nterm, j = 1; j <= stop; j++)
  {
    if ( IsGroundBus( BusName ) == false )
    {
      ref = pElem.Terminals[j].BusRef;
      StartFreeInstance( GeoPrf, "PositionPoint", GetDevUuid( PosPt, pElem.ParentClass.Name + "." + pElem.LocalName, j ) );
      UuidNode( GeoPrf, "PositionPoint.Location", geoUUID );
      IntegerNode( GeoPrf, "PositionPoint.sequenceNumber", j );
      StringNode( GeoPrf, "PositionPoint.xPosition", floattostr( ActiveCircuit[ActiveActor]->Buses[ref - 1].x ) );
      StringNode( GeoPrf, "PositionPoint.yPosition", floattostr( ActiveCircuit[ActiveActor]->Buses[ref - 1].y ) );
      EndInstance( GeoPrf, "PositionPoint" );
    }
    BusName = pElem.NextBus;
  }*/
}


void WriteReferenceTerminals( TDSSCktElement pElem, TUuid RefUuid, double norm = 0.0, double emerg = 0.0 )
{
  int Nterm = 0, j = 0, ref = 0;
  String BusName, TermName, LimitName;
  TUuid TermUuid, LimiTUuid;
  TOpLimitObject* pLimit = NULL;/**
  Nterm = pElem.Nterms;
  BusName = pElem.FirstBus;
  for ( int stop = Nterm, j = 1; j <= stop; j++)
  {
    if ( IsGroundBus( BusName ) == false )
    {
      ref = pElem.Terminals[j].BusRef;
      TermName = pElem.Name + "_T" + IntToStr( j );
      TermUuid = GetTermUuid( pElem, j );
      StartFreeInstance( FunPrf, "Terminal", TermUuid );
      StringNode( FunPrf, "IdentifiedObject.mRID", UUIDToCIMString( TermUuid ) );
      StringNode( FunPrf, "IdentifiedObject.name", TermName );
      UuidNode( FunPrf, "Terminal.ConductingEquipment", RefUuid );
      IntegerNode( FunPrf, "ACDCTerminal.sequenceNumber", j );
      FD->WriteCimLn( TopoPrf, Format( "  <cim:Terminal.ConnectivityNode rdf:resource=\"#%s\"/>", VECTOROFCONST(( ActiveCircuit[ActiveActor]->Buses[ref - 1].CIM_ID )) ) );
      if ( ( j == 1 ) && ( norm > 0.0 ) )
      {
        if ( emerg < norm )
          emerg = norm;
        LimitName = GetOpLimIName( norm, emerg );
        pLimit = GetOpLimit( LimitName );
        if ( pLimit == NULL )
        {
          pLimit = new TOpLimitObject( norm, emerg );
          pLimit->LocalName = LimitName;
          pLimit->uuid = GetDevUuid( OpLimI, LimitName, 0 );
          AddOpLimit( pLimit );
        }
        LimiTUuid = GetDevUuid( OpLimI, LimitName, 0 );
        UuidNode( FunPrf, "ACDCTerminal.OperationalLimitSet", LimiTUuid );
      }
      EndInstance( FunPrf, "Terminal" );
    }
    BusName = pElem.NextBus;
  }*/
}

void WriteTerminals( TDSSCktElement pElem, TUuid geoUUID, TUuid crsUUID, double norm = 0.0, double emerg = 0.0 )
{/**
  WriteReferenceTerminals( pElem, pElem.uuid, norm, emerg );
  WritePositions( pElem, geoUUID, crsUUID );*/
}

void VbaseNode( ProfileChoice prf, TDSSCktElement pElem )
{
  int j = 0;
  j = pElem.Terminals[1 - 1].BusRef;
  UuidNode( prf, "ConductingEquipment.BaseVoltage", GetBaseVUuid( sqrt( 3.0 ) * ActiveCircuit[ActiveActor]->Buses[j - 1]->kVBase ) );
}

void WriteXfmrCode( TXfmrCodeObj pXfCd )
{
  TNamedObject pName, pBank;
  double ratShort = 0.0, ratEmerg = 0.0, val = 0.0, Zbase = 0.0;
  int i = 0, j = 0, Seq = 0;
  pName = TNamedObject( "dummy" );
  pBank = TNamedObject( "dummy" );
  /*# with pXfCd do *//**
  {
    pBank.LocalName = pXfCd.Name + "_PowerXfInfo";
    pBank.uuid = GetDevUuid( XfInfo, pXfCd.Name, 1 );
    StartInstance( CatPrf, "PowerTransformerInfo", pBank );
    EndInstance( CatPrf, "PowerTransformerInfo" );
    StartInstance( CatPrf, "TransformerTankInfo", pXfCd );
    RefNode( CatPrf, "TransformerTankInfo.PowerTransformerInfo", pBank );
    EndInstance( CatPrf, "TransformerTankInfo" );
    ratShort = double( NormMaxHKVA ) / Winding[1].kva;
    ratEmerg = double( EmergMaxHKVA ) / Winding[1].kva;
    for ( int stop = NumWindings, i = 1; i <= stop; i++)
    {
      Zbase = Winding[i].kvll;
      Zbase = 1000.0 * Zbase * Zbase / Winding[1].kva;
      pName.LocalName = pXfCd.Name + "_" + IntToStr( i );
      pName.uuid = GetDevUuid( WdgInf, pXfCd.Name, i );
      StartInstance( CatPrf, "TransformerEndInfo", pName );
      RefNode( CatPrf, "TransformerEndInfo.TransformerTankInfo", pXfCd );
      IntegerNode( CatPrf, "TransformerEndInfo.endNumber", i );
      if ( pXfCd.Fnphases < 3 )
      {
        WindingConnectionEnum( CatPrf, 'I' );
        if ( ( i == 3 ) && ( Winding[i].kvll < 0.3 ) ) // for center-tap secondary
          IntegerNode( CatPrf, "TransformerEndInfo.phaseAngleClock", 6 );
        else
          IntegerNode( CatPrf, "TransformerEndInfo.phaseAngleClock", 0 );
      }
      else
      {
        if ( Winding[i].Connection == 1 )
          WindingConnectionEnum( CatPrf, 'D' );
        else
          if ( ( Winding[i].Rneut > 0.0 ) || ( Winding[i].Xneut > 0.0 ) )
            WindingConnectionEnum( CatPrf, "Yn" );
          else
            WindingConnectionEnum( CatPrf, 'Y' );
        if ( Winding[i].Connection != Winding[1].Connection )
          IntegerNode( CatPrf, "TransformerEndInfo.phaseAngleClock", 1 );
        else
          IntegerNode( CatPrf, "TransformerEndInfo.phaseAngleClock", 0 );
      }
      DoubleNode( CatPrf, "TransformerEndInfo.ratedU", 1000 * Winding[i].kvll );
      DoubleNode( CatPrf, "TransformerEndInfo.ratedS", 1000 * Winding[i].kva );
      DoubleNode( CatPrf, "TransformerEndInfo.shortTermS", 1000 * Winding[i].kva * ratShort );
      DoubleNode( CatPrf, "TransformerEndInfo.emergencyS", 1000 * Winding[i].kva * ratEmerg );
      DoubleNode( CatPrf, "TransformerEndInfo.r", Winding[i].Rpu * Zbase );
      DoubleNode( CatPrf, "TransformerEndInfo.insulationU", 0.0 );
      EndInstance( CatPrf, "TransformerEndInfo" );
    }
    pName.LocalName = pXfCd.Name + "_" + IntToStr( 1 );
    pName.uuid = GetDevUuid( OcTest, pXfCd.Name, 1 );
    StartInstance( CatPrf, "NoLoadTest", pName );
    UuidNode( CatPrf, "NoLoadTest.EnergisedEnd", GetDevUuid( WdgInf, pXfCd.Name, 1 ) );
    DoubleNode( CatPrf, "NoLoadTest.energisedEndVoltage", 1000.0 * Winding[1].kvll );
    DoubleNode( CatPrf, "NoLoadTest.excitingCurrent", pctImag );
    DoubleNode( CatPrf, "NoLoadTest.excitingCurrentZero", pctImag );
    val = 0.01 * pctNoLoadLoss * Winding[1].kva; // losses to be in kW
    DoubleNode( CatPrf, "NoLoadTest.loss", val );
    DoubleNode( CatPrf, "NoLoadTest.lossZero", val );
    DoubleNode( CatPrf, "TransformerTest.basePower", 1000.0 * Winding[1].kva );
    DoubleNode( CatPrf, "TransformerTest.temperature", 50.0 );
    EndInstance( CatPrf, "NoLoadTest" );
    Seq = 0;
    for ( int stop = NumWindings, i = 1; i <= stop; i++)
      for ( int stop = NumWindings, j = ( i + 1 ); j <= stop; j++)
      {
        Seq++;
        pName.LocalName = pXfCd.Name + "_" + IntToStr( Seq );
        pName.uuid = GetDevUuid( ScTest, pXfCd.Name, Seq );
        StartInstance( CatPrf, "ShortCircuitTest", pName );
        UuidNode( CatPrf, "ShortCircuitTest.EnergisedEnd", GetDevUuid( WdgInf, pXfCd.Name, i ) );
         // NOTE: can insert more than one GroundedEnds for three-winding short-circuit tests
        UuidNode( CatPrf, "ShortCircuitTest.GroundedEnds", GetDevUuid( WdgInf, pXfCd.Name, j ) );
        IntegerNode( CatPrf, "ShortCircuitTest.energisedEndStep", Winding[i].NumTaps / 2 );
        IntegerNode( CatPrf, "ShortCircuitTest.groundedEndStep", Winding[j].NumTaps / 2 );
        Zbase = Winding[i].kvll;
        Zbase = 1000.0 * Zbase * Zbase / Winding[1].kva;  // all DSS impedances are on winding 1 base
        val = Xsc[Seq] * Zbase;
        DoubleNode( CatPrf, "ShortCircuitTest.leakageImpedance", val );
        DoubleNode( CatPrf, "ShortCircuitTest.leakageImpedanceZero", val );
        if ( Seq == 1 )
        { // put all the load loss on test from wdg1 to wdg2
          val = 0.01 * pctLoadLoss * Winding[1].kva; // losses are to be in kW
          DoubleNode( CatPrf, "ShortCircuitTest.loss", val );
          DoubleNode( CatPrf, "ShortCircuitTest.lossZero", val );
        }
        else
        {
          DoubleNode( CatPrf, "ShortCircuitTest.loss", 0.0 );
          DoubleNode( CatPrf, "ShortCircuitTest.lossZero", 0.0 );
        }
        DoubleNode( CatPrf, "TransformerTest.basePower", 1000.0 * Winding[i].kva );
        DoubleNode( CatPrf, "TransformerTest.temperature", 50.0 );
        EndInstance( CatPrf, "ShortCircuitTest" );
      }
  }
  pName.Free;*/
}

void WriteCableData( TCableDataObj pCab )
{
  double v1 = 0.0;
  /*# with pCab do *//**
  {
    v1 = To_Meters( RadiusUnits );
    BooleanNode( CatPrf, "WireInfo.insulated", true );
    DoubleNode( CatPrf, "WireInfo.insulationThickness", v1 * pCab.InsLayer );
    ConductorInsulationEnum( CatPrf, "crosslinkedPolyethylene" ); // TODO -  code EpsR
    CableOuterJacketEnum( CatPrf, "none" );
    CableConstructionEnum( CatPrf, "stranded" );
    BooleanNode( CatPrf, "CableInfo.isStrandFill", false ); // we don't really know this
    DoubleNode( CatPrf, "CableInfo.diameterOverCore", v1 * ( pCab.DiaIns - 2.0 * pCab.InsLayer ) );
    DoubleNode( CatPrf, "CableInfo.diameterOverInsulation", v1 * pCab.DiaIns );
    DoubleNode( CatPrf, "CableInfo.diameterOverJacket", v1 * pCab.DiaCable );
    DoubleNode( CatPrf, "CableInfo.nominalTemperature", 90.0 );  // we don't really know this
  }*/
}


void WriteTapeData( TTSDataObj pCab )
{
  double v1 = 0.0;
  /*# with pCab do *//**
  {
    v1 = To_Meters( RadiusUnits );
    DoubleNode( CatPrf, "CableInfo.diameterOverScreen", v1 * ( pCab.DiaShield - 2.0 * pCab.TapeLayer ) );
    DoubleNode( CatPrf, "TapeShieldCableInfo.tapeLap", pCab.TapeLap );
    DoubleNode( CatPrf, "TapeShieldCableInfo.tapeThickness", v1 * pCab.TapeLayer );
    CableShieldMaterialEnum( CatPrf, "copper" );
    BooleanNode( CatPrf, "CableInfo.sheathAsNeutral", true );
  }*/
}


void WriteConcData( TCNDataObj pCab )
{
  double v1 = 0.0;
  /*# with pCab do *//**
  {
    v1 = To_Meters( RadiusUnits );
    DoubleNode( CatPrf, "CableInfo.diameterOverScreen", v1 * ( pCab.DiaCable - 2.0 * pCab.DiaStrand ) );
    DoubleNode( CatPrf, "ConcentricNeutralCableInfo.diameterOverNeutral", v1 * pCab.DiaCable );
    DoubleNode( CatPrf, "ConcentricNeutralCableInfo.neutralStrandRadius", v1 * 0.5 * pCab.DiaStrand );
    DoubleNode( CatPrf, "ConcentricNeutralCableInfo.neutralStrandGmr", v1 * pCab.GmrStrand );
    v1 = To_per_Meter( ResUnits );
    DoubleNode( CatPrf, "ConcentricNeutralCableInfo.neutralStrandRDC20", v1 * pCab.RStrand );
    IntegerNode( CatPrf, "ConcentricNeutralCableInfo.neutralStrandCount", pCab.NStrand );
    BooleanNode( CatPrf, "CableInfo.sheathAsNeutral", false );
  }*/
}


void WriteWireData( TConductorDataObj pWire )
{
  double v1 = 0.0;
  /*# with pWire do *//**
  {
    StringNode( CatPrf, "WireInfo.sizeDescription", DisplayName );
    if ( CompareText( LeftStr( Name, 2 ), "AA" ) == 0 )
      ConductorMaterialEnum( CatPrf, "aluminum" );
    else
      if ( CompareText( LeftStr( Name, 4 ), "ACSR" ) == 0 )
        ConductorMaterialEnum( CatPrf, "acsr" );
      else
        if ( CompareText( LeftStr( Name, 2 ), "CU" ) == 0 )
          ConductorMaterialEnum( CatPrf, "copper" );
        else
          if ( CompareText( LeftStr( Name, 3 ), "EHS" ) == 0 )
            ConductorMaterialEnum( CatPrf, "steel" );
          else
            ConductorMaterialEnum( CatPrf, "other" );
    v1 = To_Meters( GMRUnits );
    DoubleNode( CatPrf, "WireInfo.gmr", GMR * v1 );
    v1 = To_Meters( RadiusUnits );
    DoubleNode( CatPrf, "WireInfo.radius", Radius * v1 );
    v1 = To_per_Meter( ResUnits );
    DoubleNode( CatPrf, "WireInfo.rDC20", Rdc * v1 );
    DoubleNode( CatPrf, "WireInfo.rAC25", Rac * v1 );
    DoubleNode( CatPrf, "WireInfo.rAC50", Rac * v1 );
    DoubleNode( CatPrf, "WireInfo.rAC75", Rac * v1 );
    DoubleNode( CatPrf, "WireInfo.ratedCurrent", MaxValue( VECTOROFCONST(( NormAmps, 0.0 )) ) );
    IntegerNode( CatPrf, "WireInfo.strandCount", 0 );
    IntegerNode( CatPrf, "WireInfo.coreStrandCount", 0 );
    DoubleNode( CatPrf, "WireInfo.coreRadius", 0.0 );
  }*/
}

void StartCIMFile( Textfile& F, String Filenm, ProfileChoice prf )
{
  AssignFile( F, Filenm );
  Rewrite( F );
  IOResultToException();
   WriteLn( F, "<?xml version=\"1.0\" encoding=\"utf-8\"?>" );
   WriteLn( F, "<!-- un-comment this line to enable validation" );
   WriteLn( F, "-->" );
   WriteLn( F, String( "<rdf:RDF xmlns:cim=\"" ) + CIM_NS + "#\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">" );
   WriteLn( F, "<!--" );
   WriteLn( F, "-->" );
   /*
   WriteLn( Format( "<cim:IEC61970CIMVersion rdf:ID=\"%s\">", VECTOROFCONST(( UUIDToCIMString( GetDevUuid( CIMVer, "IEC", 1 ) ) )) ) );
   WriteLn( Format( "  <cim:IEC61970CIMVersion.version>%s</cim:IEC61970CIMVersion.version>", VECTOROFCONST(( "IEC61970CIM100" )) ) );
   WriteLn( Format( "  <cim:IEC61970CIMVersion.date>%s</cim:IEC61970CIMVersion.date>", VECTOROFCONST(( "2019-04-01" )) ) );*/
   WriteLn( F, "</cim:IEC61970CIMVersion>" );
}


void ListXfmrCodes( TXfmrCode clsXfCd, String lbl ) // for debugging

{/*
  TXfmrCodeObj pXfCd;
  WriteLn( String( "xfmrcodes at " ) + lbl );
  pXfCd = clsXfCd.ElementList.Get_First();
  while ( pXfCd != NULL )
  {
    WriteLn( F, "  " + pXfCd.LocalName + " " + pXfCd.Name + " " + UUIDToString( pXfCd.uuid ) );
    pXfCd = clsXfCd.ElementList.Get_Next();
  }*/
}

// more to be added if needed in the future --$%^#---








