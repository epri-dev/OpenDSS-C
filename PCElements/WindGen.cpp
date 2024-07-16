

#pragma hdrstop

#include "WindGen.h"

#include "Circuit.h"
#include "DSSGlobals.h"
#include "Utilities.h"

using namespace std;
using namespace Arraydef;
using namespace Circuit;
using namespace CktElement;
using namespace CktElementClass;
using namespace Command;
using namespace DSSClass;
using namespace DSSClassDefs;
using namespace DSSGlobals;
using namespace DSSObject;
using namespace Dynamics;
using namespace LoadShape;
using namespace PCClass;
using namespace PCElement;
using namespace ParserDel;
using namespace System;
using namespace Ucmatrix;
using namespace Ucomplex;
using namespace WindGenUserModel;
using namespace WindGenVars;
using namespace mathutil;
using namespace Utilities;

namespace WindGen
{

TWindGenObj::TWindGenObj(DSSClass::TDSSClass* ParClass) : inherited(ParClass) {}
TWindGenObj::TWindGenObj(String ClassName) : inherited(ClassName) {}
TWindGenObj::TWindGenObj() {}


TWindGenObj* ActiveWindGenObj = nullptr;
TWindGen* WinGenClass = nullptr;
const int NumPropsThisClass = 39;  // removed Fuel variables
  // Dispatch modes
const int Default = 0;
const int LOADMODE = 1;
complex cBuffer[24/*# range 1..24*/];  // Temp buffer for calcs  24-phase WindGen?
complex CDoubleOne = {};
//    TwoPI3:Double;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Creates superstructure for all Line objects

TWindGen::TWindGen()
{
	;
	Class_Name ="WindGen";
	DSSClassType = DSSClassType + WINDGEN_ELEMENT;  // In both PCelement and Genelement list
	ActiveElement = 0;

     // Set Register names
	RegisterNames[1 - 1] ="kWh";
	RegisterNames[2 - 1] ="kvarh";
	RegisterNames[3 - 1] ="Max kW";
	RegisterNames[4 - 1] ="Max kVA";
	RegisterNames[5 - 1] ="Hours";
	RegisterNames[6 - 1] ="$";
	DefineProperties();
	std::string* slc = Slice((PropertyName), NumProperties);
	CommandList = TCommandList(slc, NumProperties);
	delete[] slc;
	CommandList.set_AbbrevAllowed(true);
	WinGenClass = this;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

TWindGen::~TWindGen()
{

    // ElementList and  CommandList freed in inherited destroy
	// inherited::Destroy();
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TWindGen::DefineProperties()
{
	NumProperties = NumPropsThisClass;
	inherited::CountProperties();   // Get inherited property count
	AllocatePropertyArrays();   /*see DSSClass*/

     // Define Property names
	PropertyName[1 - 1] ="phases";
	PropertyHelp[1 - 1] ="Number of Phases, this WindGen.  Power is evenly divided among phases.";
	PropertyName[2 - 1] ="bus1";
	PropertyHelp[2 - 1] ="Bus to which the WindGen is connected.  May include specific node specification.";
	PropertyName[3 - 1] ="kv";
	PropertyHelp[3 - 1] ="Nominal rated (1.0 per unit) voltage, kV, for WindGen. For 2- and 3-phase WindGens, specify phase-phase kV. "
	          "Otherwise, for phases=1 or phases>3, specify actual kV across each branch of the WindGen. "
	          "If wye (star), specify phase-neutral kV. "
	          "If delta or phase-phase connected, specify phase-phase kV.";
	PropertyName[4 - 1] ="kW";
	PropertyHelp[4 - 1] = String("Total base kW for the WindGen.  A positive value denotes power coming OUT of the element, ") + CRLF
	           +"which is the opposite of a load. This value is modified depending on the dispatch mode. "
	           +"Unaffected by the global load multiplier and growth curves. "
	           +"If you want there to be more generation, you must add more WindGens or change this value.";
	PropertyName[5 - 1] ="PF";
	PropertyHelp[5 - 1] = String("WindGen power factor. Default is 0.80. Enter negative for leading powerfactor ""(when kW and kvar have opposite signs.)") + CRLF
	           +"A positive power factor for a WindGen signifies that the WindGen produces vars "
	           + CRLF
	           +"as is typical for a synchronous WindGen.  Induction machines would be "
	           + CRLF
	           +"generally specified with a negative power factor.";
	PropertyName[6 - 1] ="model";
	PropertyHelp[6 - 1] = String("Integer code for the model to use for generation variation with voltage. ""Valid values are:") + CRLF
	           + CRLF
	           +"1:WindGen injects a constant kW at specified power factor."
	           + CRLF
	           +"2:WindGen is modeled as a constant admittance."
	           + CRLF
	           +"3:Const kW, constant kV.  Voltage-regulated model."
	           + CRLF
	           +"4:Const kW, Fixed Q (Q never varies)"
	           + CRLF
	           +"5:Const kW, Fixed Q(as a constant reactance)"
	           + CRLF
	           +"6:Compute load injection from User-written Model.(see usage of Xd, Xdp)";
	PropertyName[7 - 1] ="yearly";
	PropertyHelp[7 - 1] ="Dispatch shape to use for yearly-mode simulations.  Must be previously defined "
	          "as a Loadshape object. If this is not specified, a constant value is assumed (no variation). "
	          "Set to NONE to reset to no loadahape. "
	          "Nominally for 8760 simulations.  If there are fewer points in the designated shape than "
	          "the number of points in the solution, the curve is repeated.";
	PropertyName[8 - 1] ="daily";
	PropertyHelp[8 - 1] ="Dispatch shape to use for daily-mode simulations.  Must be previously defined "
	          "as a Loadshape object of 24 hrs, typically."
	          "Set to NONE to reset to no loadahape. "; // daily dispatch (hourly)
	PropertyName[9 - 1] ="duty";
	PropertyHelp[9 - 1] ="Load shape to use for duty cycle dispatch simulations such as for wind or solar generation. "
	          "Must be previously defined as a Loadshape object. "
	          "Typically would have time intervals less than 1 hr -- perhaps, in seconds. "
	          "Set to NONE to reset to no loadahape. "
	          "Designate the number of points to solve using the Set Number=xxxx command. "
	          "If there are fewer points in the actual shape, the shape is assumed to repeat.";  // as for wind generation
	PropertyName[10 - 1] =" ";
	PropertyHelp[10 - 1] =" "; // = 0 | >0
	PropertyName[11 - 1] =" ";
	PropertyHelp[11 - 1] =" ";  // = 0 | >0
	PropertyName[12 - 1] ="conn";
	PropertyHelp[12 - 1] ="={wye|LN|delta|LL}.  Default is wye.";
	PropertyName[13 - 1] ="kvar";
	PropertyHelp[13 - 1] ="Specify the base kvar.  Alternative to specifying the power factor.  Side effect: "
	          " the power factor value is altered to agree based on present value of kW.";
	PropertyName[16 - 1] ="status";
	PropertyHelp[16 - 1] ="={Fixed | Variable*}.  If Fixed, then dispatch multipliers do not apply. "
	          "The WindGen is always at full power when it is ON. "
	          " Default is Variable  (follows curves or windspeed).";  // fixed or variable
	PropertyName[17 - 1] ="class";
	PropertyHelp[17 - 1] ="An arbitrary integer number representing the class of WindGen so that WindGen values may "
	          "be segregated by class."; // integer
	PropertyName[18 - 1] ="Vpu";
	PropertyHelp[18 - 1] ="Per Unit voltage set point for Model = 3  (Regulated voltage model).  Default is 1.0 pu. "; // per unit set point voltage for power flow model
	PropertyName[19 - 1] ="maxkvar";
	PropertyHelp[19 - 1] ="Maximum kvar limit for Model = 3.  Defaults to twice the specified load kvar.  "
	          "Always reset this if you change PF or kvar properties.";
	PropertyName[20 - 1] ="minkvar";
	PropertyHelp[20 - 1] ="Minimum kvar limit for Model = 3. Enter a negative number if WindGen can absorb vars."
	          " Defaults to negative of Maxkvar.  Always reset this if you change PF or kvar properties.";
	PropertyName[21 - 1] ="pvfactor";
	PropertyHelp[21 - 1] ="Deceleration factor for P-V WindGen model (Model=3).  Default is 0.1. "
	          "If the circuit converges easily, you may want to use a higher number such as 1.0. "
	          "Use a lower number if solution diverges. Use Debugtrace=yes to create a file that will "
	          "trace the convergence of a WindGen model.";
	PropertyName[22 - 1] ="debugtrace";
	PropertyHelp[22 - 1] ="{Yes | No }  Default is no.  Turn this on to capture the progress of the WindGen model "
	          "for each iteration.  Creates a separate file for each WindGen named \"GEN_name.CSV\".";
	PropertyName[23 - 1] ="Vminpu";
	PropertyHelp[23 - 1] ="Default = 0.90.  Minimum per unit voltage for which the Model is assumed to apply. "
	          "Below this value, the Windgen model reverts to a constant impedance model. For model 7, the current is "
	          "limited to the value computed for constant power at Vminpu.";
	PropertyName[24 - 1] ="Vmaxpu";
	PropertyHelp[24 - 1] ="Default = 1.10.  Maximum per unit voltage for which the Model is assumed to apply. "
	          "Above this value, the Windgen model reverts to a constant impedance model.";
	PropertyName[25 - 1] ="forceon";
	PropertyHelp[25 - 1] ="{Yes | No}  Forces WindGen ON despite requirements of other dispatch modes. "
	          "Stays ON until this property is set to NO, or an internal algorithm cancels the forced ON state.";
	PropertyName[26 - 1] ="kVA";
	PropertyHelp[26 - 1] ="kVA rating of electrical machine. Defaults to 1.2* kW if not specified. Applied to machine or inverter definition for Dynamics mode solutions. ";
	PropertyName[27 - 1] ="MVA";
	PropertyHelp[27 - 1] ="MVA rating of electrical machine.  Alternative to using kVA=.";
	PropertyName[28 - 1] ="Xd";
	PropertyHelp[28 - 1] ="Per unit synchronous reactance of machine. Presently used only for Thevinen impedance for power flow calcs of user models (model=6). "
	          "Typically use a value 0.4 to 1.0. Default is 1.0";
	PropertyName[29 - 1] ="Xdp";
	PropertyHelp[29 - 1] ="Per unit transient reactance of the machine.  Used for Dynamics mode and Fault studies.  Default is 0.27."
	          "For user models, this value is used for the Thevinen/Norton impedance for Dynamics Mode.";
	PropertyName[30 - 1] ="Xdpp";
	PropertyHelp[30 - 1] ="Per unit subtransient reactance of the machine.  Used for Harmonics. Default is 0.20.";
	PropertyName[31 - 1] ="H";
	PropertyHelp[31 - 1] ="Per unit mass constant of the machine.  MW-sec/MVA.  Default is 1.0.";
	PropertyName[32 - 1] ="D";
	PropertyHelp[32 - 1] ="Damping constant.  Usual range is 0 to 4. Default is 1.0.  Adjust to get damping";
	PropertyName[33 - 1] ="UserModel";
	PropertyHelp[33 - 1] ="Name of DLL containing user-written model, which computes the terminal currents for Dynamics studies, "
	          "overriding the default model.  Set to \"none\" to negate previous setting.";
	PropertyName[34 - 1] ="UserData";
	PropertyHelp[34 - 1] ="String (in quotes or parentheses) that gets passed to user-written model for defining the data required for that model.";
	PropertyName[35 - 1] ="ShaftModel";
	PropertyHelp[35 - 1] ="Name of user-written DLL containing a Shaft model, which models the prime mover and determines the power on the shaft for Dynamics studies. "
	          "Models additional mass elements other than the single-mass model in the DSS default model. Set to \"none\" to negate previous setting.";
	PropertyName[36 - 1] ="ShaftData";
	PropertyHelp[36 - 1] ="String (in quotes or parentheses) that gets passed to user-written shaft dynamic model for defining the data for that model.";
	PropertyName[37 - 1] ="DutyStart";
	PropertyHelp[37 - 1] ="Starting time offset [hours] into the duty cycle shape for this WindGen, defaults to 0";
	PropertyName[38 - 1] ="Balanced";
	PropertyHelp[38 - 1] ="{Yes | No*} Default is No.  For Model=7, force balanced current only for 3-phase WindGens. Force zero- and negative-sequence to zero.";
	PropertyName[39 - 1] ="XRdp";
	PropertyHelp[39 - 1] ="Default is 20. X/R ratio for Xdp property for FaultStudy and Dynamic modes.";

      /*Removed Fuel-related variables 40-44 from Generator model*/
	ActiveProperty = NumPropsThisClass - 1;
	inherited::DefineProperties();  // Add defs of inherited properties to bottom of list

     // Override default help string
	PropertyHelp[NumPropsThisClass + 1 - 1] ="Name of harmonic voltage or current spectrum for this WindGen. "
	          "Voltage behind Xd\" for machine - default. Current injection for inverter. "
	          "Default value is \"default\", which is defined when the DSS starts.";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int TWindGen::NewObject(const String ObjName)
{
	int result = 0;
    // Make a new WindGen and add it to WindGen class list
	/*# with ActiveCircuit[ActiveActor] do */
	{
		
		ActiveCircuit[ActiveActor]->Set_ActiveCktElement(new TWindGenObj(this, ObjName));
		result = AddObjectToList(ActiveDSSObject[ActiveActor]);
	}
	return result;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TWindGen::SetNcondsForConnection()
{
	/*# with ActiveWindGenObj do */
	{
		auto with0 = ActiveWindGenObj;
		switch(with0->Connection)
		{
			case 	0:
			with0->Set_Nconds(with0->Fnphases + 1);
			break;
			case 	1:
			switch(with0->Fnphases)
			{
				case 	1:
				 case 2:
				with0->Set_Nconds(with0->Fnphases + 1);
				break; // L-L and Open-delta
				default:
				with0->Set_Nconds(with0->Fnphases);
				break;
			}
			break;
			default:
			  ;
			break;
		}
	}
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// Accepts
//    delta or LL           (Case insensitive)
//    Y, wye, or LN

void TWindGen::InterpretConnection(const String s)
{
	String TestS;
	/*# with ActiveWindGenObj do */
	{
		auto with0 = ActiveWindGenObj;
		TestS = LowerCase(s);
		switch(TestS[1])
		{
			case 	L'y':
			 case L'w':
			with0->Connection = 0;
			break;  /*Wye*/
			case 	L'd':
			with0->Connection = 1;
			break;  /*Delta or line-Line*/
			case 	L'l':
			switch(TestS[2])
			{
				case 	L'n':
				with0->Connection = 0;
				break;
				case 	L'l':
				with0->Connection = 1;
				break;
				default:
				  ;
				break;
			}
			break;
			default:
			  ;
			break;
		}
		SetNcondsForConnection();

            /*VBase is always L-N voltage unless 1-phase device or more than 3 phases*/
		/*# with WindGenVars do */
		{
			auto& with1 = with0->WindGenVars;
			switch(with0->Fnphases)
			{ /*CASE Connection OF
              1: VBase := kVWindGenBase * 1000.0 ;
              Else*/
				case 	2: case 3:
				with0->VBase = with1.kVWindGenBase * InvSQRT3x1000;
				break;    // L-N Volts
				default:
				with0->VBase = with1.kVWindGenBase * 1000.0;   // Just use what is supplied
				break;
			}
		}
            /*End;*/
		with0->VBase95 = with0->Vminpu * with0->VBase;
		with0->VBase105 = with0->Vmaxpu * with0->VBase;
		with0->Yorder = with0->Fnconds * with0->Fnterms;
		with0->Set_YprimInvalid(ActiveActor,true);
	}
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int InterpretDispMode(const String s)
{
	int result = 0;
	switch(LowerCase(s)[0])
	{
		case 	L'l':
		result = LOADMODE;
		break;
		default:
		result = Default;
		break;
	}
	return result;
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int TWindGen::Edit(int ActorID)
{
	int result = 0;
	int i = 0;
	int ParamPointer = 0;
	String ParamName;
	String Param;
  // continue parsing with contents of Parser
	ActiveWindGenObj = (TWindGenObj*) ElementList.Get_Active();
	ActiveCircuit[ActorID]->Set_ActiveCktElement(ActiveWindGenObj);
	result = 0;
	/*# with ActiveWindGenObj do */
	{
		auto with0 = ActiveWindGenObj;
		ParamPointer = 0;
		ParamName = Parser[ActorID]->GetNextParam();
		Param = Parser[ActorID]->MakeString_();
		while(Param.size() > 0)
		{
			if(ParamName.size() == 0)
				++ParamPointer;
			else
				ParamPointer = CommandList.Getcommand(ParamName);
			if((ParamPointer > 0) && (ParamPointer <= NumProperties))
				with0->Set_PropertyValue((PropertyIdxMap)[ParamPointer - 1],Param);
			else
				DoSimpleMsg(String("Unknown parameter \"") + ParamName
	           +"\" for WindGen \""
	           + with0->get_Name()
	           +"\"", 560);
			if(ParamPointer > 0)
				switch((PropertyIdxMap)[ParamPointer - 1])
				{
					case 	0:
					DoSimpleMsg(String("Unknown parameter \"") + ParamName
	           +"\" for Object \""
	           + Class_Name
	           +"."
	           + with0->get_Name()
	           +"\"", 561);
					break;
					case 	1:
					with0->Set_NPhases(Parser[ActorID]->MakeInteger_());
					break; // num phases
					case 	2:
					with0->SetBus(1, Param);
					break;
					case 	3:
					with0->Set_PresentkV(Parser[ActorID]->MakeDouble_());
					break;
					case 	4:
					with0->kWBase = Parser[ActorID]->MakeDouble_();
					break;
					case 	5:
					with0->PFNominal = Parser[ActorID]->MakeDouble_();
					break;
					case 	6:
					with0->GenModel = Parser[ActorID]->MakeInteger_();
					break;
					case 	7:
					with0->YearlyShape = Param;
					break;
					case 	8:
					with0->DailyDispShape = Param;
					break;
					case 	9:
					with0->DutyShape = Param;
					break;
					case 	10:
					;
					break;
					case 	11:
					;
					break;
					case 	12:
					InterpretConnection(Param);
					break;
					case 	13:
					with0->Set_Presentkvar(Parser[ActorID]->MakeDouble_());
					break;
					case 	14:
					;
					break;
					case 	15:
					;
					break;
					case 	16:
					if(LowerCase(&(Param[1])) =="f")
						with0->IsFixed = true;
					else
						with0->IsFixed = false;
					break;
					case 	17:
					with0->GenClass = Parser[ActorID]->MakeInteger_();
					break;
					case 	18:
					with0->Vpu = Parser[ActorID]->MakeDouble_();
					break;
					case 	19:
					with0->kvarMax = Parser[ActorID]->MakeDouble_();
					break;
					case 	20:
					with0->kvarMin = Parser[ActorID]->MakeDouble_();
					break;
					case 	21:
					with0->PVFactor = Parser[ActorID]->MakeDouble_();
					break;  //decelaration factor
					case 	22:
					with0->DebugTrace = InterpretYesNo(Param);
					break;
					case 	23:
					with0->Vminpu = Parser[ActorID]->MakeDouble_();
					break;
					case 	24:
					with0->Vmaxpu = Parser[ActorID]->MakeDouble_();
					break;
					case 	25:
					with0->FForcedON = InterpretYesNo(Param);
					break;
					case 	26:
					with0->WindGenVars.kVArating = Parser[ActorID]->MakeDouble_();
					break;
					case 	27:
					with0->WindGenVars.kVArating = Parser[ActorID]->MakeDouble_() * 1000.0;
					break;  // 'MVA';
					case 	28:
					with0->WindGenVars.puXd = Parser[ActorID]->MakeDouble_();
					break;
					case 	29:
					with0->WindGenVars.puXdp = Parser[ActorID]->MakeDouble_();
					break;
					case 	30:
					with0->WindGenVars.puXdpp = Parser[ActorID]->MakeDouble_();
					break;
					case 	31:
					with0->WindGenVars.Hmass = Parser[ActorID]->MakeDouble_();
					break;
					case 	32:
					with0->WindGenVars.Dpu = Parser[ActorID]->MakeDouble_();
					break;
					case 	33:
					with0->UserModel->Set_Name(Parser[ActorID]->MakeString_());
					break;  // Connect to user written models
					case 	34:
					with0->UserModel->Set_Edit(Parser[ActorID]->MakeString_());
					break;  // Send edit string to user model
					case 	35:
					with0->ShaftModel->Set_Name(Parser[ActorID]->MakeString_());
					break;
					case 	36:
					with0->ShaftModel->Set_Edit(Parser[ActorID]->MakeString_());
					break;
					case 	37:
					with0->DutyStart = Parser[ActorID]->MakeDouble_();
					break;
					case 	38:
					with0->ForceBalanced = InterpretYesNo(Param);
					break;
					case 	39:
					with0->WindGenVars.XRdp = Parser[ActorID]->MakeDouble_();
					break;  // X/R for dynamics model

           // Inherited parameters
					default:
					inherited::ClassEdit(ActiveWindGenObj, ParamPointer - NumPropsThisClass);
					break;
				}
			if(ParamPointer > 0)
				switch((PropertyIdxMap)[ParamPointer - 1])
				{
					case 	1:
					SetNcondsForConnection();
					break;  // Force Reallocation of terminal info

            // keep kvar nominal up to date with kW and PF
					case 	4: case 5:
					with0->SyncUpPowerQuantities();
					break;

            // if a model 3 WindGen added, force calc of dQdV
					case 	6:
					if(with0->GenModel == 3)
						ActiveCircuit[ActorID]->Solution->SolutionInitialized = false;
					break;

    /*Set shape objects;  returns nil if not valid*/
     /*Sets the kW and kvar properties to match the peak kW demand from the Loadshape*/
					case 	7:
					{
						with0->YearlyShapeObj = ((TLoadShapeObj*) LoadShapeClass[ActorID]->Find(with0->YearlyShape));
						if(ASSIGNED(with0->YearlyShapeObj))
							/*# with YearlyShapeObj do */
							{
								auto with1 = with0->YearlyShapeObj;
								if(with1->UseActual)
									with0->SetkWkvar(with1->MaxP, with1->MaxQ);
							}
					}
					break;
					case 	8:
					{
						with0->DailyDispShapeObj = ((TLoadShapeObj*) LoadShapeClass[ActorID]->Find(with0->DailyDispShape));
						if(ASSIGNED(with0->DailyDispShapeObj))
							/*# with DailyDispShapeObj do */
							{
								auto with2 = with0->DailyDispShapeObj;
								if(with2->UseActual)
									with0->SetkWkvar(with2->MaxP, with2->MaxQ);
							}
					}
					break;
					case 	9:
					{
						with0->DutyShapeObj = ((TLoadShapeObj*) LoadShapeClass[ActorID]->Find(with0->DutyShape));
						if(ASSIGNED(with0->DutyShapeObj))
							/*# with DutyShapeObj do */
							{
								auto with3 = with0->DutyShapeObj;
								if(with3->UseActual)
									with0->SetkWkvar(with3->MaxP, with3->MaxQ);
							}
					}
					break;
					case 	22:
					if(with0->DebugTrace)
					{
						int stop = 0;
						AssignFile(with0->Tracefile, GetOutputDirectory() +"WINDGEN_" + with0->get_Name() +".CSV");
						Rewrite(with0->Tracefile);
						IOResultToException();
						Write(with0->Tracefile,"t, Iteration, LoadMultiplier, Mode, LoadModel, GenModel, dQdV, Avg_Vpu, Vdiff, MQnominalperphase, MPnominalperphase, CurrentType");
						for(stop = with0->Get_NPhases(), i = 1; i <= stop; i++)
						{
							Write(with0->Tracefile, String(", |Iinj") + IntToStr(i) +"|");
						}
						for(stop = with0->Get_NPhases(), i = 1; i <= stop; i++)
						{
							Write(with0->Tracefile, String(", |Iterm") + IntToStr(i) +"|");
						}
						for(stop = with0->Get_NPhases(), i = 1; i <= stop; i++)
						{
							Write(with0->Tracefile, String(", |Vterm") + IntToStr(i) +"|");
						}
						Write(with0->Tracefile,",Vthev, Theta");
						WriteLn(with0->Tracefile);
						CloseFile(with0->Tracefile);
					}
					break;
					case 	26: case 27:
					with0->kVANotSet = false;
					break;
					default:
					  ;
					break;
				}
			ParamName = Parser[ActorID]->GetNextParam();
			Param = Parser[ActorID]->MakeString_();
		}
		with0->RecalcElementData(ActorID);
		with0->Set_YprimInvalid(ActorID,true);
	}
	return result;
}

//----------------------------------------------------------------------------

int TWindGen::MakeLike(const String OtherWindGenName)
{
	int result = 0;
	TWindGenObj* OtherWindGen = nullptr;
	int i = 0;
	result = 0;
   /*See if we can find this line name in the present collection*/
	OtherWindGen = ((TWindGenObj*) Find(OtherWindGenName));
	if(OtherWindGen != nullptr)
		/*# with ActiveWindGenObj do */
		{
			auto with0 = ActiveWindGenObj;
			int stop = 0;
			if(with0->Fnphases != OtherWindGen->Fnphases)
			{
				with0->Set_NPhases(OtherWindGen->Fnphases);
				with0->Set_Nconds(with0->Fnphases);  // Forces reallocation of terminal stuff
				with0->Yorder = with0->Fnconds * with0->Fnterms;
				with0->Set_YprimInvalid(ActiveActor,true);
			}
			with0->WindGenVars.kVWindGenBase = OtherWindGen->WindGenVars.kVWindGenBase;
			with0->VBase = OtherWindGen->VBase;
			with0->Vminpu = OtherWindGen->Vminpu;
			with0->Vmaxpu = OtherWindGen->Vmaxpu;
			with0->VBase95 = OtherWindGen->VBase95;
			with0->VBase105 = OtherWindGen->VBase105;
			with0->kWBase = OtherWindGen->kWBase;
			with0->kvarBase = OtherWindGen->kvarBase;
			with0->WindGenVars.Pnominalperphase = OtherWindGen->WindGenVars.Pnominalperphase;
			with0->PFNominal = OtherWindGen->PFNominal;
			with0->WindGenVars.Qnominalperphase = OtherWindGen->WindGenVars.Qnominalperphase;
			with0->varMin = OtherWindGen->varMin;
			with0->varMax = OtherWindGen->varMax;
			with0->Connection = OtherWindGen->Connection;
     //  Rneut          := OtherWindGen.Rneut;
      // Xneut          := OtherWindGen.Xneut;
			with0->YearlyShape = OtherWindGen->YearlyShape;
			with0->YearlyShapeObj = OtherWindGen->YearlyShapeObj;
			with0->DailyDispShape = OtherWindGen->DailyDispShape;
			with0->DailyDispShapeObj = OtherWindGen->DailyDispShapeObj;
			with0->DutyShape = OtherWindGen->DutyShape;
			with0->DutyShapeObj = OtherWindGen->DutyShapeObj;
			with0->DutyStart = OtherWindGen->DutyStart;
			with0->GenClass = OtherWindGen->GenClass;
			with0->GenModel = OtherWindGen->GenModel;
			with0->IsFixed = OtherWindGen->IsFixed;
			with0->WindGenVars.VTarget = OtherWindGen->WindGenVars.VTarget;
			with0->Vpu = OtherWindGen->Vpu;
			with0->kvarMax = OtherWindGen->kvarMax;
			with0->kvarMin = OtherWindGen->kvarMin;
			with0->FForcedON = OtherWindGen->FForcedON;
			with0->kVANotSet = OtherWindGen->kVANotSet;
			with0->WindGenVars.kVArating = OtherWindGen->WindGenVars.kVArating;
			with0->WindGenVars.puXd = OtherWindGen->WindGenVars.puXd;
			with0->WindGenVars.puXdp = OtherWindGen->WindGenVars.puXdp;
			with0->WindGenVars.puXdpp = OtherWindGen->WindGenVars.puXdpp;
			with0->WindGenVars.Hmass = OtherWindGen->WindGenVars.Hmass;
			with0->WindGenVars.Theta = OtherWindGen->WindGenVars.Theta;
			with0->WindGenVars.Speed = OtherWindGen->WindGenVars.Speed;
			with0->WindGenVars.w0 = OtherWindGen->WindGenVars.w0;
			with0->WindGenVars.dSpeed = OtherWindGen->WindGenVars.dSpeed;
			with0->WindGenVars.D = OtherWindGen->WindGenVars.D;
			with0->WindGenVars.Dpu = OtherWindGen->WindGenVars.Dpu;
			with0->WindGenVars.XRdp = OtherWindGen->WindGenVars.XRdp;
			with0->UserModel->FName = OtherWindGen->UserModel->FName;  // Connect to user written models
			with0->ShaftModel->FName = OtherWindGen->ShaftModel->FName;
			ClassMakeLike(OtherWindGen);
			for(stop = with0->ParentClass->NumProperties, i = 1; i <= stop; i++)
			{
				(with0->FPropertyValue)[i - 1] = (OtherWindGen->FPropertyValue)[i - 1];
			}
			result = 1;
		}
	else
		DoSimpleMsg(String("Error in Load MakeLike: \"") + OtherWindGenName
	           +"\" Not Found.", 562);
	return result;
}

//----------------------------------------------------------------------------

int TWindGen::Init(int Handle, int ActorID)
{
	int result = 0;
	TWindGenObj* P = nullptr;
	if(Handle == 0)  // init all
	{
		P = (TWindGenObj*) ElementList.Get_First();
		while((P != nullptr))
		{
			P->Randomize(0);
			P = (TWindGenObj*) ElementList.Get_Next();
		}
	}
	else
	{
		Set_Active(Handle);
		P = ((TWindGenObj*) GetActiveObj());
		P->Randomize(0);
	}
	DoSimpleMsg("Need to implement TWindGen.Init", -1);
	result = 0;
	return result;
}

/*--------------------------------------------------------------------------*/  // Force all EnergyMeters in the circuit to reset

void TWindGen::ResetRegistersAll(int ActorID)
{
	TWindGenObj* pGen = nullptr;
	pGen = ((TWindGenObj*) ActiveCircuit[ActorID]->WindGens.Get_First());
	while((pGen != nullptr))
	{
		pGen->ResetRegisters();
		pGen = ((TWindGenObj*) ActiveCircuit[ActorID]->WindGens.Get_Next());
	}
}

/*--------------------------------------------------------------------------*/  // Force all EnergyMeters in the circuit to take a sample

void TWindGen::SampleAll(int ActorID)
{
	TWindGenObj* pGen = nullptr;
	pGen = ((TWindGenObj*) ActiveCircuit[ActorID]->WindGens.Get_First());
	while(pGen != nullptr)
	{
		if(pGen->Get_Enabled())
			pGen->TakeSample(ActorID);
		pGen = ((TWindGenObj*) ActiveCircuit[ActorID]->WindGens.Get_Next());
	}
}

//----------------------------------------------------------------------------

TWindGenObj::TWindGenObj(TDSSClass* ParClass, const String SourceName)
 : inherited(ParClass),
			Model7MaxPhaseCurr(0.0),
			Model7LastAngle(0.0),
			DebugTrace(false),
			DeltaQMax(0.0),
			DQDV(0.0),
			DQDVSaved(0.0),
			FForcedON(false),
			FirstSampleAfterReset(false),
			IsFixed(false),
			WindGenSolutionCount(0),
			GenFundamental(0.0),
			GenON(false),
			GenSwitchOpen(false),
			kVANotSet(false),
			LastGrowthFactor(0.0),
			LastYear(0),
			OpenWindGenSolutionCount(0),
			PVFactor(0.0),
			RandomMult(0.0),
			Reg_Hours(0),
			Reg_kvarh(0),
			Reg_kWh(0),
			Reg_MaxkVA(0),
			Reg_MaxkW(0),
			Reg_Price(0),
			UserModel(nullptr),
			ShaftModel(nullptr),
			V_Avg(0.0),
			V_Remembered(0.0),
			var_Remembered(0.0),
			varBase(0.0),
			varMax(0.0),
			varMin(0.0),
			VBase(0.0),
			VBase105(0.0),
			VBase95(0.0),
			YPrimOpenCond(nullptr),
			YQFixed(0.0),
			ShapeIsActual(false),
			ForceBalanced(false),
			Connection(0),
			DailyDispShapeObj(nullptr),
			DutyShapeObj(nullptr),
			DutyStart(0.0),
			GenClass(0),
			GenModel(0),
			kvarBase(0.0),
			kvarMax(0.0),
			kvarMin(0.0),
			kWBase(0.0),
			PFNominal(0.0),
			Vpu(0.0),
			Vmaxpu(0.0),
			Vminpu(0.0),
			GenActive(false),
			YearlyShapeObj(nullptr)
{
	int i;

	Set_Name(LowerCase(SourceName));
	DSSObjType = ParClass->DSSClassType; // + WINDGEN_ELEMENT;  // In both PCelement and Genelement list
	Set_NPhases(3);
	Fnconds = 4;  // defaults to wye
	Yorder = 0;  // To trigger an initial allocation
	Set_NTerms(1);  // forces allocations
	kWBase = 1000.0;
	kvarBase = 60.0;
	kvarMax = kvarBase * 2.0;
	kvarMin = -kvarMax;
	PFNominal = 0.88;
	YearlyShape ="";
	YearlyShapeObj = nullptr;  // if YearlyShapeobj = nil then the load alway stays nominal * global multipliers
	DailyDispShape ="";
	DailyDispShapeObj = nullptr;  // if DaillyShapeobj = nil then the load alway stays nominal * global multipliers
	DutyShape ="";
	DutyShapeObj = nullptr;  // if DutyShapeobj = nil then the load alway stays nominal * global multipliers
	DutyStart = 0.0;
	Connection = 0;    // Wye (star)
	GenModel = 1;  /*Typical fixed kW negative load*/
	GenClass = 1;
	LastYear = 0;
	LastGrowthFactor = 1.0;
	DQDVSaved = 0.0;  // Initialize this here.  Allows WindGens to be turned off and on
	WindGenSolutionCount = -1;  // For keep track of the present solution in Injcurrent calcs
	OpenWindGenSolutionCount = -1;
	YPrimOpenCond = nullptr;
	WindGenVars.kVWindGenBase = 12.47;
	Vpu = 1.0;
	WindGenVars.VTarget = 1000.0 * Vpu * WindGenVars.kVWindGenBase / SQRT3;  /*Line-to-Neutral target*/
	VBase = 7200.0;
	Vminpu = 0.90;
	Vmaxpu = 1.10;
	VBase95 = Vminpu * VBase;
	VBase105 = Vmaxpu * VBase;
	Yorder = Fnterms * Fnconds;
	RandomMult = 1.0;
	IsFixed = false;

     /*Machine rating stuff*/
	WindGenVars.kVArating = kWBase * 1.2;
	kVANotSet = true;  // Flag for default value for kVA
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		with0.puXd = 1.0;
		with0.puXdp = 0.28;
		with0.puXdpp = 0.20;
		with0.XD = with0.puXd * Sqr(with0.kVWindGenBase) * 1000.0L / with0.kVArating;
		with0.Xdp = with0.puXdp * Sqr(with0.kVWindGenBase) * 1000.0L / with0.kVArating;
		with0.Xdpp = with0.puXdpp * Sqr(with0.kVWindGenBase) * 1000.0L / with0.kVArating;
		with0.Hmass = 1.0;       //  W-sec/VA rating
		with0.Theta = 0.0;
		with0.w0 = TwoPi * BaseFrequency;
		with0.Speed = 0.0;
		with0.dSpeed = 0.0;
		with0.D = 1.0;
		with0.XRdp = 20.0;
	}

     /*Advertise Genvars struct as public*/
	PublicDataStruct = ((void*) &WindGenVars);
	PublicDataSize = sizeof(TWindGenVars);
	UserModel = new TWindGenUserModel(&WindGenVars);
	ShaftModel = new TWindGenUserModel(&WindGenVars);

  // Register values inherited from Generator model
	Reg_kWh = 1 - 1;
	Reg_kvarh = 2 - 1;
	Reg_MaxkW = 3 - 1;
	Reg_MaxkVA = 4 - 1;
	Reg_Hours = 5  - 1;
	Reg_Price = 6 - 1;
	PVFactor = 0.1;
	DebugTrace = false;
	FForcedON = false;
	GenSwitchOpen = false;
	ShapeIsActual = false;
	ForceBalanced = false;
	Spectrum ="defaultgen";  // override base class
	GenActive = true;   // variable to use if needed
	InitPropertyValues(0);
	RecalcElementData(ActiveActor);

	for (i = 0; i < NumGenRegisters; i++)
	{
		Registers[i] = 0.0;
		Derivatives[i] = 0.0;
	}
}


//----------------------------------------------------------------------------

TWindGenObj::~TWindGenObj()
{
	delete YPrimOpenCond;
	delete UserModel;
	delete ShaftModel;
	// inherited::Destroy();
}


//----------------------------------------------------------------------------

void TWindGenObj::Randomize(int Opt)
{
	switch(Opt)
	{
		case 	0:
		RandomMult = 1.0;
		break;
		case 	GAUSSIAN:
		RandomMult = Gauss(YearlyShapeObj->Get_Mean(), YearlyShapeObj->Get_StdDev());
		break;
		case 	UNIFORM:
		RandomMult = (double) Random();
		break;  // number between 0 and 1.0
		case 	LOGNORMAL:
		RandomMult = QuasiLogNormal(YearlyShapeObj->Get_Mean());
		break;
		default:
		  ;
		break;
	}
}

//----------------------------------------------------------------------------

void TWindGenObj::CalcDailyMult(double hr)
{
	if(DailyDispShapeObj != nullptr)
	{
		ShapeFactor = DailyDispShapeObj->GetMult(hr);
		ShapeIsActual = DailyDispShapeObj->UseActual;
	}
	else
	ShapeFactor = CDoubleOne;  // Default to no daily variation
}


//----------------------------------------------------------------------------

void TWindGenObj::CalcDutyMult(double hr)
{
	if(DutyShapeObj != nullptr)
	{
		ShapeFactor = DutyShapeObj->GetMult(hr + DutyStart);
		ShapeIsActual = DutyShapeObj->UseActual;
	}
	else
	CalcDailyMult(hr);  // Default to Daily Mult if no duty curve specified
}

//----------------------------------------------------------------------------

void TWindGenObj::CalcYearlyMult(double hr)
{

/*Yearly curve is assumed to be hourly only*/
	if(YearlyShapeObj != nullptr)
	{
		ShapeFactor = YearlyShapeObj->GetMult(hr);
		ShapeIsActual = YearlyShapeObj->UseActual;
	}
	else
	ShapeFactor = CDoubleOne;  // Defaults to no variation
}



//----------------------------------------------------------------------------

void TWindGenObj::SetNominalGeneration(int ActorID)
{
	double Factor = 0.0;
	bool GenOn_Saved = false;
	GenOn_Saved = GenON;
	ShapeFactor = CDoubleOne;
    // Check to make sure the generation is ON
	/*# with ActiveCircuit[ActorID], ActiveCircuit[ActorID].Solution do */
	{
		
		auto with1 = ActiveCircuit[ActorID]->Solution;
		if(!(with1->IsDynamicModel || with1->IsHarmonicModel))     // Leave WindGen in whatever state it was prior to entering Dynamic mode
		{
			GenON = true;   // Init to on then check if it should be off
 //       IF NOT FForcedON
 //       THEN CASE DispatchMode of
 //****          LOADMODE: IF (DispatchValue > 0.0)   AND (WindGenDispatchReference < DispatchValue)  THEN GenON := FALSE;
 //****          PRICEMODE:IF (DispatchValue > 0.0)   AND (PriceSignal < DispatchValue) THEN GenON := FALSE;
 //       END;
		}
		if(!GenON)
         // If WindGen is OFF enter as tiny resistive load (.0001 pu) so we don't get divide by zero in matrix
		{
			WindGenVars.Pnominalperphase = -0.1 * kWBase / Fnphases;
          // Pnominalperphase   := 0.0;
			WindGenVars.Qnominalperphase = 0.0;
		}
		else
    // WindGen is on, compute it's nominal watts and vars
		{
			/*# with Solution do */
			{
				auto with2 = ActiveCircuit[ActorID]->Solution;
				if(IsFixed)
				{
					Factor = 1.0;   // for fixed WindGens, set constant
				}
				else
				{
					switch(with2->Get_SolMode())
					{
						case 	SNAPSHOT:
						Factor = ActiveCircuit[ActorID]->GenMultiplier * 1.0;
						break;
						case 	DAILYMODE:
						{
							Factor = ActiveCircuit[ActorID]->GenMultiplier; // Daily dispatch curve
							CalcDailyMult(with2->DynaVars.dblHour);
						}
						break;
						case 	YEARLYMODE:
						{
							Factor = ActiveCircuit[ActorID]->GenMultiplier;
							CalcYearlyMult(with2->DynaVars.dblHour);
						}
						break;
						case 	DUTYCYCLE:
						{
							Factor = ActiveCircuit[ActorID]->GenMultiplier;
							CalcDutyMult(with2->DynaVars.dblHour);
						}
						break;   // General sequential time simulation
						case 	GENERALTIME: case DYNAMICMODE:
						{
							Factor = ActiveCircuit[ActorID]->GenMultiplier;
                                       // This mode allows use of one class of load shape
							switch(ActiveCircuit[ActorID]->ActiveLoadShapeClass)
							{
								case 	USEDAILY:
								CalcDailyMult(with2->DynaVars.dblHour);
								break;
								case 	USEYEARLY:
								CalcYearlyMult(with2->DynaVars.dblHour);
								break;
								case 	USEDUTY:
								CalcDutyMult(with2->DynaVars.dblHour);
								break;
								default:     // default to 1 + j1 if not known
								ShapeFactor = CDoubleOne;
								break;
							}
						}
						break;
						case 	MONTECARLO1: case MONTEFAULT: case FAULTSTUDY:
						Factor = ActiveCircuit[ActorID]->GenMultiplier * 1.0;
						break;
						case 	MONTECARLO2: case MONTECARLO3: case LOADDURATION1: case LOADDURATION2:
						{
							Factor = ActiveCircuit[ActorID]->GenMultiplier;
							CalcDailyMult(with2->DynaVars.dblHour);
						}
						break;
						case 	PEAKDAY:
						{
							Factor = ActiveCircuit[ActorID]->GenMultiplier;
							CalcDailyMult(with2->DynaVars.dblHour);
						}
						break;
						case 	AUTOADDFLAG:
						Factor = 1.0;
						break;
						default:
						Factor = 1.0;
						break;
					}
				}
			}
			if(!(with1->IsDynamicModel || with1->IsHarmonicModel))         //******
			{
				if(ShapeIsActual)
					WindGenVars.Pnominalperphase = 1000.0 * ShapeFactor.re / Fnphases;
				else
					WindGenVars.Pnominalperphase = 1000.0 * kWBase * Factor * ShapeFactor.re / Fnphases;
				/*# with WindGenVars do */
				{
					auto& with3 = WindGenVars;
					if(GenModel == 3)   /* Just make sure present value is reasonable*/
					{
						if(with3.Qnominalperphase > varMax)
							with3.Qnominalperphase = varMax;
						else
						{
							if(with3.Qnominalperphase < varMin)
								with3.Qnominalperphase = varMin;
						}
					}
					else

                   /* for other WindGen models*/
					{
						if(ShapeIsActual)
							with3.Qnominalperphase = 1000.0 * ShapeFactor.im / Fnphases;
						else
							with3.Qnominalperphase = 1000.0 * kvarBase * Factor * ShapeFactor.im / Fnphases;
					}
				}
			}
		} /*ELSE GenON*/
		if(!(with1->IsDynamicModel || with1->IsHarmonicModel))       //******
		{
			switch(GenModel)
			{
				case 	6:
				Yeq = cinv(cmplx(0.0, -WindGenVars.XD));
				break;  // Gets negated in CalcYPrim
				default:
				/*# with WindGenVars do */
				{
					auto& with4 = WindGenVars;
					Yeq = cdivreal(cmplx(with4.Pnominalperphase, -with4.Qnominalperphase), Sqr(VBase));
				}   // Vbase must be L-N for 3-phase
				if(Vminpu != 0.0)  // at 95% voltage
					Yeq95 = cdivreal(Yeq, Sqr(Vminpu));
				else
					Yeq95 = Yeq; // Always a constant Z model
				if(Vmaxpu != 0.0)   // at 105% voltage
					Yeq105 = cdivreal(Yeq, Sqr(Vmaxpu));
				else
					Yeq105 = Yeq;
				break;
			}

          /* When we leave here, all the Yeq's are in L-N values*/
			if(GenModel == 7)
				/*# with WindGenVars do */
				{
					auto& with5 = WindGenVars;
					PhaseCurrentLimit = cdivreal(cmplx(with5.Pnominalperphase, -with5.Qnominalperphase), VBase95);
					Model7MaxPhaseCurr = cabs(PhaseCurrentLimit);
				}
		}
	}  /*With ActiveCircuit[ActiveActor]*/

   // If WindGen state changes, force re-calc of Y matrix
	if(GenON != GenOn_Saved)
		Set_YprimInvalid(ActorID,true);
}

//----------------------------------------------------------------------------

void TWindGenObj::RecalcElementData(int ActorID)
{
	VBase95 = Vminpu * VBase;
	VBase105 = Vmaxpu * VBase;
	varBase = 1000.0 * kvarBase / Fnphases;
	varMin = 1000.0 * kvarMin / Fnphases;
	varMax = 1000.0 * kvarMax / Fnphases;

    /*Populate data structures used for interchange with user-written models.*/
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		with0.XD = with0.puXd * 1000.0 * Sqr(with0.kVWindGenBase) / with0.kVArating;
		with0.Xdp = with0.puXdp * 1000.0 * Sqr(with0.kVWindGenBase) / with0.kVArating;
		with0.Xdpp = with0.puXdpp * 1000.0 * Sqr(with0.kVWindGenBase) / with0.kVArating;
		with0.Conn = Connection;
		with0.NumPhases = Fnphases;
		with0.NumConductors = Fnconds;
	}
	SetNominalGeneration(ActorID);

    /*Now check for errors.  If any of these came out nil and the string was not nil, give warning*/
	if(CompareText(YearlyShape,"none") == 0)
		YearlyShape ="";
	if(CompareText(DailyDispShape,"none") == 0)
		DailyDispShape ="";
	if(CompareText(DutyShape,"none") == 0)
		DutyShape ="";
	if(YearlyShapeObj == nullptr)
	{
		if(YearlyShape.size() > 0)
			DoSimpleMsg(String("WARNING! Yearly load shape: \"") + YearlyShape
	           +"\" Not Found.", 563);
	}
	if(DailyDispShapeObj == nullptr)
	{
		if(DailyDispShape.size() > 0)
			DoSimpleMsg(String("WARNING! Daily load shape: \"") + DailyDispShape
	           +"\" Not Found.", 564);
	}
	if(DutyShapeObj == nullptr)
	{
		if(DutyShape.size() > 0)
			DoSimpleMsg(String("WARNING! Duty load shape: \"") + DutyShape
	           +"\" Not Found.", 565);
	}
	SpectrumObj = ((TSpectrumObj*) SpectrumClass[ActorID]->Find(Spectrum));
	if(SpectrumObj == nullptr)
		DoSimpleMsg(String("ERROR! Spectrum \"") + Spectrum +"\" Not Found.", 566);
	YQFixed = -varBase / Sqr(VBase);   //10-17-02  Fixed negative sign
	WindGenVars.VTarget = Vpu * 1000.0 * WindGenVars.kVWindGenBase;
	if(Fnphases > 1)
		WindGenVars.VTarget = WindGenVars.VTarget / SQRT3;

    // Initialize to Zero - defaults to PQ WindGen
    // Solution object will reset after circuit modifications
	DQDV = DQDVSaved;         // for Model = 3
	DeltaQMax = (varMax - varMin) * 0.10;  // Limit to 10% of range
	InjCurrent = (pComplexArray)realloc(InjCurrent, sizeof(complex) * Yorder);

    /*Update any user-written models*/
	if(UserModel->Get_Exists())
		UserModel->FUpdateModel();
	if(ShaftModel->Get_Exists())
		ShaftModel->FUpdateModel();
}

//----------------------------------------------------------------------------

void TWindGenObj::CalcYPrimMatrix(TcMatrix* Ymatrix, int ActorID)
{
	complex Y = {};
	complex Yij = {};
	int i = 0;
	int j = 0;
	double FreqMultiplier = 0.0;
	FYprimFreq = ActiveCircuit[ActorID]->Solution->get_FFrequency();
	FreqMultiplier = FYprimFreq / BaseFrequency;
	/*# with ActiveCircuit[ActorID].Solution do */
	{
		auto with0 = ActiveCircuit[ActorID]->Solution;
		if(with0->IsDynamicModel || with0->IsHarmonicModel)
		{
			int stop = 0;
			if(GenON)   // L-N value computed in initialization routines
				Y = Yeq;
			else
				Y = cmplx(EPSILON, 0.0);
			if(Connection == 1)
				Y = cdivreal(Y, 3.0); // Convert to delta impedance
			Y.im = Y.im / FreqMultiplier;
			Yij = cnegate(Y);
			for(stop = Fnphases, i = 1; i <= stop; i++)
			{
				switch(Connection)
				{
					case 	0:
					{
						Ymatrix->SetElement(i, i, Y);
						Ymatrix->AddElement(Fnconds, Fnconds, Y);
						Ymatrix->SetElemsym(i, Fnconds, Yij);
					}
					break;   /*Delta connection*/
					case 	1:
					{
						int stop1 = 0;
						Ymatrix->SetElement(i, i, Y);
						Ymatrix->AddElement(i, i, Y);  // put it in again
						for(stop1 = i - 1, j = 1; j <= stop1; j++)
						{
							Ymatrix->SetElemsym(i, j, Yij);
						}
					}
					break;
					default:
					  ;
					break;
				}
			}

      /**** Removed Neutral / Neutral may float

       IF Connection = 0 Then   With Ymatrix Do  // Take care of neutral issues
         Begin
           AddElement(Fnconds, Fnconds, YNeut);  // Add in user specified Neutral Z, if any
           // Bump up neutral-ground in case neutral ends up floating
           SetElement(Fnconds, Fnconds, CmulReal(GetElement(Fnconds, Fnconds), 1.000001));
         End;

      */
		}
		else
  //  Regular power flow WindGen model
		
       /*Yeq is always expected as the equivalent line-neutral admittance*/
		{
			Y = cnegate(Yeq);  // negate for generation    Yeq is L-N quantity

       // ****** Need to modify the base admittance for real harmonics calcs
			Y.im = Y.im / FreqMultiplier;
			switch(Connection)
			{
				case 	0:
				/*# with Ymatrix do */
				{
					auto with1 = Ymatrix; // WYE
					int stop = 0;
					Yij = cnegate(Y);
					for(stop = Fnphases, i = 1; i <= stop; i++)
					{
						with1->SetElement(i, i, Y);
						with1->AddElement(Fnconds, Fnconds, Y);
						with1->SetElemsym(i, Fnconds, Yij);
					}
				}
				break;
				case 	1:
				/*# with Ymatrix do */
				{
					auto with2 = Ymatrix;  // Delta  or L-L
					int stop = 0;
					Y = cdivreal(Y, 3.0); // Convert to delta impedance
					Yij = cnegate(Y);
					for(stop = Fnphases, i = 1; i <= stop; i++)
					{
						j = i + 1;
						if(j > Fnconds)
							j = 1;  // wrap around for closed connections
						with2->AddElement(i, i, Y);
						with2->AddElement(j, j, Y);
						with2->AddElemsym(i, j, Yij);
					}
				}
				break;
				default:
				  ;
				break;
			}
		}
	}  /*ELSE IF Solution.mode*/
}


//----------------------------------------------------------------------------

void TWindGenObj::CalcYPrim(int ActorID)
{
	int i = 0;

     // Build only shunt Yprim
     // Build a dummy Yprim Series so that CalcV does not fail
	int stop = 0;
	if(Get_YprimInvalid(ActorID,0))
	{
		if(YPrim_Shunt != nullptr)
			delete YPrim_Shunt;
		YPrim_Shunt = new TcMatrix(Yorder);
		if(YPrim_Series != nullptr)
			delete YPrim_Series;
		YPrim_Series = new TcMatrix(Yorder);
		if(YPrim != nullptr)
			delete YPrim;
		YPrim = new TcMatrix(Yorder);
	}
	else
	{
		YPrim_Shunt->Clear();
		YPrim_Series->Clear();
		YPrim->Clear();
	}
	if(ActiveCircuit[ActorID]->Solution->LoadModel == POWERFLOW)
     
        // 12-7-99 we'll start with Yeq in system matrix
	{
		SetNominalGeneration(ActorID);
		CalcYPrimMatrix(YPrim_Shunt, ActorID);
	}
	else


         // ADMITTANCE model wanted
	{
		SetNominalGeneration(ActorID);
		CalcYPrimMatrix(YPrim_Shunt, ActorID);
	}

     // Set YPrim_Series based on diagonals of YPrim_shunt  so that CalcVoltages doesn't fail
	for(stop = Yorder, i = 1; i <= stop; i++)
	{
		YPrim_Series->SetElement(i, i, cmulreal(YPrim_Shunt->GetElement(i, i), 1.0e-10));
	}
	YPrim->CopyFrom(YPrim_Shunt);

     // Account for Open Conductors
	inherited::CalcYPrim(ActorID);
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

 /*Add the current into the proper location according to connection*/

 /*Reverse of similar routine in load  (Cnegates are switched)*/

void TWindGenObj::StickCurrInTerminalArray(pComplexArray TermArray, const complex& Curr, int i)
{
	int j = 0;
	switch(Connection)
	{
		case 	0:  //Wye
		{
			caccum((TermArray)[i - 1], Curr);
			caccum((TermArray)[Fnconds - 1], cnegate(Curr)); // Neutral
		}
		break; //DELTA
		case 	1:
		{
			caccum((TermArray)[i - 1], Curr);
			j = i + 1;
			if(j > Fnconds)
				j = 1;
			caccum((TermArray)[j - 1], cnegate(Curr));
		}
		break;
		default:
		  ;
		break;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::WriteTraceRecord(const String s, int ActorID)
{
	int i = 0;
	try
	{
		if(!InShowResults)
		{
			int stop = 0;
			Append(Tracefile);
			IOResultToException();
			{ Write(Tracefile, Format("%-.g, %d, %-.g, ", 
				ActiveCircuit[ActorID]->Solution->DynaVars.T + ActiveCircuit[ActorID]->Solution->DynaVars.intHour * 3600.0, 
					ActiveCircuit[ActorID]->Solution->Iteration,
					ActiveCircuit[ActorID]->get_FLoadMultiplier())); 

			Write(Tracefile, GetSolutionModeID()); 
			Write(Tracefile,", "); Write(Tracefile, 
				GetLoadModel()); Write(Tracefile,", "); 
			Write(Tracefile, GenModel, 0); 
			Write(Tracefile,", "); 
			Write(Tracefile, DQDV, 8, 0); 
			Write(Tracefile,", "); 
			Write(Tracefile, (V_Avg * 0.001732 / WindGenVars.kVWindGenBase), 8, 3); 
			Write(Tracefile,", "); 
			Write(Tracefile, (WindGenVars.VTarget - V_Avg), 9, 1); 
			Write(Tracefile,", "); 
			Write(Tracefile, (WindGenVars.Qnominalperphase * 3.0 / 1.0e6), 8, 2); 
			Write(Tracefile,", "); 
			Write(Tracefile, (WindGenVars.Pnominalperphase * 3.0 / 1.0e6), 8, 2); 
			Write(Tracefile,", "); 
			Write(Tracefile, s); 
			Write(Tracefile,", "); }

			for(stop = Get_NPhases(), i = 1; i <= stop; i++)
			{
				{ Write(Tracefile, (cabs((InjCurrent)[i - 1])), 8, 1); Write(Tracefile,", "); }
			}
			for(stop = Get_NPhases(), i = 1; i <= stop; i++)
			{
				{ Write(Tracefile, (cabs((Iterminal)[i - 1])), 8, 1); Write(Tracefile,", "); }
			}
			for(stop = Get_NPhases(), i = 1; i <= stop; i++)
			{
				{ Write(Tracefile, (cabs((Vterminal)[i - 1])), 8, 1); Write(Tracefile,", "); }
			}
			{ Write(Tracefile, WindGenVars.VthevMag, 8, 1); Write(Tracefile,", "); Write(Tracefile, WindGenVars.Theta * 180.0 / DSSGlobals::PI); }
			WriteLn(Tracefile);
			CloseFile(Tracefile);
		}
	}
	catch (...)
	{
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


/*Compute total terminal current for Constant PQ*/

void TWindGenObj::DoConstantPQGen(int ActorID)
{
	int i = 0;
	complex Curr = {};
	complex V = {};
	double Vmag = 0.0;
//   V012,I012 :Array[0..2] of Complex;
//   Iabc :Array[1..3] of Complex;

     //Treat this just like the Load model
	int stop = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array
	ZeroITerminal();

    /*****   Tried this but couldn't get it to work
    CASE Fnphases of

    3:With Genvars Do Begin     // Use Symmetrical Components
          Phase2SymComp(Vterminal, @V012);   // Vterminal is L-N voltages here
                         // Phase2SymComp(InjCurrent, @I012);   // Vterminal is L-G voltages here
          V := V012[1]; // Positive sequence L-N voltage
          Vmag := Cabs(V012[1]);

           { IF   VMag <= VBase95
            THEN Curr := Cnegate(Cmul(Yeq95, V))  // Below 95% (Vminpu) use an impedance model
            ELSE If VMag > VBase105
            THEN Curr := Cnegate(Cmul(Yeq105, V))  // above 105% (Vmaxpu) use an impedance model
            }
            IF   (VMag <= VBase95) or (VMag > VBase105) THEN    Curr := Conjg( Cdiv( CurrentLimit, CDivReal(V, -Vmag)) )
            ELSE With Genvars Do Curr := Conjg(Cdiv(Cmplx(-Pnominalperphase, -Qnominalperphase), V));    // Current INTO pos seq model

         I012[1] := Curr;  // Pos sequence current into the terminal

          If Connection=1 Then I012[0] := CZERO  Else I012[0] := Cdiv(V012[0], cmplx(0.0, xdpp));
          I012[2] := Cdiv(V012[2], cmplx(0.0, xdpp));

          // Negative and Zero Sequence Contributions
         SymComp2Phase(@Iabc, @I012);    // Iabc now desired terminal current
         IF DebugTrace Then Begin
             Append(TraceFile);
			 IOResultToException();
             Write(TraceFile,Format('V1=%-.5g, /_%-.5g, ',[Cabs(V), CDang(V)]));
             Write(TraceFile,Format('I1=%-.5g, /_%-.5g, ',[Cabs(Curr), CDang(Curr)]));
             Write(TraceFile,'Iabc=');
             For i := 1 to 3 Do Write(TraceFile,Format('%-.5g, /_%-.5g, ',[ Cabs(Iabc[i]), CDang(Iabc[i])]));
             Writeln(TraceFile);
             CloseFile(TraceFile);
         End;

          For i := 1 to 3 Do Begin
            ITerminal^[i] := Iabc[i];  // Put into Terminal array directly because we have computed line current above
            Caccum(InjCurrent^[i], Cnegate(Iabc[i]));  // subtract in
            If Connection=0 Then Begin
               Caccum(Iterminal^[Fnconds], Cnegate(Iabc[i]));  // Neutral
               Caccum(InjCurrent^[Fnconds], Iabc[i]);  // Neutral
            End;
          End;
          IterminalUpdated := TRUE;  // so that we con't have to recompute for a report
      End
    ELSE
    ****/
	CalcVTerminalPhase(ActorID); // get actual voltage across each phase of the load
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		V = (Vterminal)[i - 1];
		Vmag = cabs(V);
		switch(Connection)
		{
			case 	0:  /*Wye*/
			{
				if(Vmag <= VBase95)  // Below 95% use an impedance model
					Curr = cmul(Yeq95, V);
				else
				{
					if(Vmag > VBase105)  // above 105% use an impedance model
						Curr = cmul(Yeq105, V);
					else
						/*# with WindGenVars do */
						{
							auto& with0 = WindGenVars;
							Curr = conjg(cdiv(cmplx(with0.Pnominalperphase, with0.Qnominalperphase), V));
						}  // Between 95% -105%, constant PQ
				}
			}
			break;  /*Delta*/
			case 	1:
			{
				switch(Fnphases)
				{
					case 	2:
					 case 3:
					Vmag = Vmag / SQRT3;
					break;  // L-N magnitude
					
                        /*leave Vmag as is*/
					default:
					  ;
					break;
				}
				if(Vmag <= VBase95)  // Below 95% use an impedance model
					Curr = cmul(cdivreal(Yeq95, 3.0), V);
				else
				{
					if(Vmag > VBase105)  // above 105% use an impedance model
						Curr = cmul(cdivreal(Yeq105, 3.0), V);
					else
						/*# with WindGenVars do */
						{
							auto& with1 = WindGenVars;
							Curr = conjg(cdiv(cmplx(with1.Pnominalperphase, with1.Qnominalperphase), V));
						}  // Between 95% -105%, constant PQ
				}
			}
			break;
			default:
			  ;
			break;
		}
		StickCurrInTerminalArray(&(Iterminal[0]), cnegate(Curr), i);  // Put into Terminal array taking into account connection
		set_ITerminalUpdated(true, ActorID);
		StickCurrInTerminalArray(InjCurrent, Curr, i);  // Put into Terminal array taking into account connection
	}
    /*END;*/
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::DoConstantZGen(int ActorID)
{
	int i = 0;
	complex Curr = {};
	complex YEQ2 = {};

// Assume Yeq is kept up to date
	int stop = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array
	CalcVTerminalPhase(ActorID); // get actual voltage across each phase of the load
	ZeroITerminal();
	if(Connection == 0)
		YEQ2 = Yeq;
	else
		YEQ2 = cdivreal(Yeq, 3.0);
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		Curr = cmul(YEQ2, (Vterminal)[i - 1]);   // Yeq is always line to neutral
		StickCurrInTerminalArray(&(Iterminal[0]), cnegate(Curr), i);  // Put into Terminal array taking into account connection
		set_ITerminalUpdated(true, ActorID);
		StickCurrInTerminalArray(InjCurrent, Curr, i);  // Put into Terminal array taking into account connection
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

/*Compute total terminal current for Constant P,|V|*/

// Constant P, constant |V|

void TWindGenObj::DoPVTypeGen(int ActorID)
{
	int i = 0;
	double DQ = 0.0;
	complex Curr = {};
	int stop = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array
	CalcVTerminalPhase(ActorID); // get actual voltage across each phase of the WindGen
	ZeroITerminal();

    // Guess at a new var output value
	V_Avg = 0.0;
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		V_Avg = V_Avg + cabs((Vterminal)[i - 1]);
	}
	if(Connection == 1)
		V_Avg = V_Avg / (SQRT3 * Fnphases);
	else
		V_Avg = V_Avg / Fnphases;

   // 12-9-99 added empirical 0.7 factor to improve iteration
   // 12-17-99 changed to 0.1 because first guess was consistently too high
	DQ = PVFactor * DQDV * (WindGenVars.VTarget - V_Avg);   // Vtarget is L-N
	if(Abs((int) DQ) > DeltaQMax)
	{
		if(DQ < 0.0)
			DQ = -DeltaQMax;
		else
			DQ = DeltaQMax;
	}
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		with0.Qnominalperphase = with0.Qnominalperphase + DQ;
	}

   /* Test Limits*/
	/*# with WindGenVars do */
	{
		auto& with1 = WindGenVars;
		int stop = 0;
		if(with1.Qnominalperphase > varMax)
			with1.Qnominalperphase = varMax;
		else
		{
			if(with1.Qnominalperphase < varMin)
				with1.Qnominalperphase = varMin;

       // Compute injection currents using W and var values
       // Do not use comstant Z models outside normal range
       // Presumably the var source will take care of the voltage problems
		}
		for(stop = Fnphases, i = 1; i <= stop; i++)
		{
			Curr = conjg(cdiv(cmplx(with1.Pnominalperphase, with1.Qnominalperphase), (Vterminal)[i - 1]));
			StickCurrInTerminalArray(&(Iterminal[0]), cnegate(Curr), i);  // Put into Terminal array taking into account connection
			set_ITerminalUpdated(true, ActorID);
			StickCurrInTerminalArray(InjCurrent, Curr, i);  // Put into Terminal array taking into account connection
		}
	} /*With*/
}


// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


/*Compute total terminal current for Fixed Q*/
// Constant P, Fixed Q  Q is always kvarBase

void TWindGenObj::DoFixedQGen(int ActorID)
{
	int i = 0;
	complex Curr = {};
	complex V = {};
	double Vmag = 0.0;
	int stop = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array
	CalcVTerminalPhase(ActorID); // get actual voltage across each phase of the load
	ZeroITerminal();
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		V = (Vterminal)[i - 1];
		Vmag = cabs(V);
		switch(Connection)
		{
			case 	0:
			{
				if(Vmag <= VBase95)  // Below 95% use an impedance model
					Curr = cmul(cmplx(Yeq95.re, YQFixed), V);
				else
				{
					if(Vmag > VBase105)  // above 105% use an impedance model
						Curr = cmul(cmplx(Yeq105.re, YQFixed), V);
					else
						Curr = conjg(cdiv(cmplx(WindGenVars.Pnominalperphase, varBase), V));
				}
			}
			break;
			case 	1:
			{
				switch(Fnphases)
				{
					case 	2:
					 case 3:
					Vmag = Vmag / SQRT3;
					break;  // L-N magnitude
					
                    /*leave Vmag as is*/
					default:
					  ;
					break;
				}
				if(Vmag <= VBase95)  // Below 95% use an impedance model
					Curr = cmul(cmplx(Yeq95.re / 3.0, YQFixed / 3.0), V);
				else
				{
					if(Vmag > VBase105)  // above 105% use an impedance model
						Curr = cmul(cmplx(Yeq105.re / 3.0, YQFixed / 3.0), V);
					else
						Curr = conjg(cdiv(cmplx(WindGenVars.Pnominalperphase, varBase), V));
				}
			}
			break;
			default:
			  ;
			break;
		}
		StickCurrInTerminalArray(&(Iterminal[0]), cnegate(Curr), i);  // Put into Terminal array taking into account connection
		set_ITerminalUpdated(true, ActorID);
		StickCurrInTerminalArray(InjCurrent, Curr, i);  // Put into Terminal array taking into account connection
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


/*Compute total terminal current for */
// Constant P, Fixed Q  Q is always a fixed Z derived from kvarBase

void TWindGenObj::DoFixedQZGen(int ActorID)
{
	int i = 0;
	complex Curr = {};
	complex V = {};
	double Vmag = 0.0;
	int stop = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array
	CalcVTerminalPhase(ActorID); // get actual voltage across each phase of the load
	ZeroITerminal();
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		V = (Vterminal)[i - 1];
		Vmag = cabs(V);
		switch(Connection)
		{
			case 	0:
			{
				if(Vmag <= VBase95)  // Below 95% use an impedance model
					Curr = cmul(cmplx(Yeq95.re, YQFixed), V);
				else
				{
					if(Vmag > VBase105)
						Curr = cmul(cmplx(Yeq105.re, YQFixed), V);
					else
					{
						Curr = conjg(cdiv(cmplx(WindGenVars.Pnominalperphase, 0.0), V)); // P component of current
						caccum(Curr, cmul(cmplx(0.0, YQFixed), V));  // add in Q component of current
					}
				}
			}
			break;
			case 	1:
			{
				switch(Fnphases)
				{
					case 	2:
					 case 3:
					Vmag = Vmag / SQRT3;
					break;  // L-N magnitude
					
                      /*leave Vmag as is*/
					default:
					  ;
					break;
				}
				if(Vmag <= VBase95)  // Below 95% use an impedance model
					Curr = cmul(cmplx(Yeq95.re / 3.0, YQFixed / 3.0), V);
				else
				{
					if(Vmag > VBase105)
						Curr = cmul(cmplx(Yeq105.re / 3.0, YQFixed / 3.0), V);
					else
					{
						Curr = conjg(cdiv(cmplx(WindGenVars.Pnominalperphase, 0.0), V)); // P component of current
						caccum(Curr, cmul(cmplx(0.0, YQFixed / 3.0), V));  // add in Q component of current
					}
				}
			}
			break;
			default:
			  ;
			break;
		}
		StickCurrInTerminalArray(&(Iterminal[0]), cnegate(Curr), i);  // Put into Terminal array taking into account connection
		set_ITerminalUpdated(true, ActorID);
		StickCurrInTerminalArray(InjCurrent, Curr, i);  // Put into Terminal array taking into account connection
	} /*FOR*/
}
// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

/*Compute total terminal Current from User-written model*/

void TWindGenObj::DoUserModel(int ActorID)
{
	int i = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array
	if(UserModel->Get_Exists())    // Check automatically selects the usermodel if true

         //AppendToEventLog('Wnominal=', Format('%-.5g',[Pnominalperphase]));
	{
		UserModel->FCalc(&(Vterminal[0]), &(Iterminal[0]));
		set_ITerminalUpdated(true, ActorID);
		/*# with ActiveCircuit[ActorID].Solution do */
		{
			auto with0 = ActiveCircuit[ActorID]->Solution;          // Negate currents from user model for power flow WindGen model
			int stop = 0;
			for(stop = Fnconds, i = 1; i <= stop; i++)
			{
				caccum((InjCurrent)[i - 1], cnegate((Iterminal)[i - 1]));
			}
		}
	}
	else
	{
		DoSimpleMsg(String("WindGen.") + get_Name()
	           +" model designated to use user-written model, but user-written model is not defined.", 567);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

/*Compute total terminal current for Constant PQ, but limit to max current below
 Vminpu*/

void TWindGenObj::DoCurrentLimitedPQ(int ActorID)
{
	int i = 0;
	complex PhaseCurr	= CZero;
	complex DeltaCurr	= CZero;
	complex VLN			= CZero;
	complex VLL			= CZero;
	double	VmagLN = 0.0;
	double	VmagLL = 0.0;
	complex V012[4] = { CZero, CZero, CZero, CZero, };  // Sequence voltages

     //Treat this just like the Load model
	int stop = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array
	CalcVTerminalPhase(ActorID); // get actual voltage across each phase of the load
	if(ForceBalanced && (Fnphases == 3))    // convert to pos-seq only
	{
		Phase2SymComp(&(Vterminal[0]), &V012[0]);
		V012[0] = CZero; // Force zero-sequence voltage to zero
		V012[2] = CZero; // Force negative-sequence voltage to zero
		SymComp2Phase(&(Vterminal[0]), &V012[0]);  // Reconstitute Vterminal as balanced
	}
	ZeroITerminal();
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		switch(Connection)
		{
			case 	0:
			{
				VLN = (Vterminal)[i - 1];   // VTerminal is LN for this connection
				VmagLN = cabs(VLN);
				/*# with WindGenVars do */
				{
					auto& with0 = WindGenVars;
					PhaseCurr = conjg(cdiv(cmplx(with0.Pnominalperphase, with0.Qnominalperphase), VLN));
				}
				if(cabs(PhaseCurr) > Model7MaxPhaseCurr)
					PhaseCurr = conjg(cdiv(PhaseCurrentLimit, cdivreal(VLN, VmagLN)));
				StickCurrInTerminalArray(&(Iterminal[0]), cnegate(PhaseCurr), i);  // Put into Terminal array taking into account connection
				set_ITerminalUpdated(true, ActorID);
				StickCurrInTerminalArray(InjCurrent, PhaseCurr, i);  // Put into Terminal array taking into account connection
			}
			break;
			case 	1:
			{
				VLL = (Vterminal)[i - 1];     // VTerminal is LL for this connection
				VmagLL = cabs(VLL);
				switch(Fnphases)
				{
					case 	2:
					 case 3:   // 2 or 3 phase WindGen model 7
					{
						/*# with WindGenVars do */
						{
							auto& with1 = WindGenVars;
							DeltaCurr = conjg(cdiv(cmplx(with1.Pnominalperphase, with1.Qnominalperphase), VLL));
						}
						if(cabs(DeltaCurr) * SQRT3 > Model7MaxPhaseCurr)
							DeltaCurr = conjg(cdiv(PhaseCurrentLimit, cdivreal(VLL, VmagLL / SQRT3)));
					}
					break;  // 1-phase WindGen model 7
					default:
					/*# with WindGenVars do */
					{
						auto& with2 = WindGenVars;
						DeltaCurr = conjg(cdiv(cmplx(with2.Pnominalperphase, with2.Qnominalperphase), VLL));
					}
					if(cabs(DeltaCurr) > Model7MaxPhaseCurr)
						DeltaCurr = conjg(cdiv(PhaseCurrentLimit, cdivreal(VLL, VmagLL)));
					break;
				}
				StickCurrInTerminalArray(&(Iterminal[0]), cnegate(DeltaCurr), i);  // Put into Terminal array taking into account connection
				set_ITerminalUpdated(true, ActorID);
				StickCurrInTerminalArray(InjCurrent, DeltaCurr, i);  // Put into Terminal array taking into account connection
			}
			break;
			default:
			  ;
			break;
		}
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


/*Compute Total Current and add into InjTemp*/

void TWindGenObj::DoDynamicMode(int ActorID)
{
	int i = 0;
	complex V012[4] = { CZero, CZero, CZero, CZero, };
	complex I012[4] = { CZero, CZero, CZero, CZero, };
	int stop = 0;
	CalcYPrimContribution(InjCurrent, ActorID);  // Init InjCurrent Array  and computes VTerminal L-N

   /*Inj = -Itotal (in) - Yprim*Vtemp*/
	switch(GenModel)
	{
		case 	6:
		if(UserModel->Get_Exists())       // auto selects model
			   /*We have total currents in Iterminal*/
			{
				UserModel->FCalc(&(Vterminal[0]), &(Iterminal[0]));  // returns terminal currents in Iterminal
			}
		else
		{
			DoSimpleMsg("Dynamics model missing for WindGen." + get_Name() + " ", 5671);
			SolutionAbort = true;
		}
		break;
		default:
		switch(Fnphases)
		{
			case 	1:
			/*# with WindGenVars do */
			{
				auto& with0 = WindGenVars;  /*No user model, use default Thevinen equivalent for standard WindGen model*/
                   // 1-phase WindGens have 2 conductors
				switch(GenModel)
				{
					case 	7:  // simple inverter model
                                  // Assume inverter stays in phase with terminal voltage
					{
						CalcVthev_Dyn_Mod7(csub((Vterminal)[1 - 1], (Vterminal)[2 - 1]));
					}
					break;
					default:
					CalcVthev_Dyn();  // Update for latest phase angle
					break;
				}
				(Iterminal)[1 - 1] = cdiv(csub(csub((Vterminal)[1 - 1], Vthev), (Vterminal)[2 - 1]), with0.Zthev);  // ZThev is based on Xd'
				if(GenModel == 7)
				{
					if(cabs((Iterminal)[1 - 1]) > Model7MaxPhaseCurr)   // Limit the current but keep phase angle
						(Iterminal)[1 - 1] = ptocomplex(topolar(Model7MaxPhaseCurr, cang((Iterminal)[1 - 1])));
				}
				(Iterminal)[2 - 1] = cnegate((Iterminal)[1 - 1]);
			}
			break;
			case 	3:
			/*# with WindGenVars do */
			{
				auto& with1 = WindGenVars;
				Phase2SymComp(&(Vterminal[0]), &V012[0]);
				switch(GenModel)
				{
					case 	7:  // simple inverter model
                                // Positive Sequence Contribution to Iterminal
                                // Assume inverter stays in phase with pos seq voltage
                                // and pos seq current is limited
					{
						CalcVthev_Dyn_Mod7(V012[1]);

                                // Positive Sequence Contribution to Iterminal
                                // Ref Frame here is all L-N
						I012[1] = cdiv(csub(V012[1], Vthev), with1.Zthev); // ZThev is based on Xd'
						if(cabs(I012[1]) > Model7MaxPhaseCurr)  // Limit the current but keep phase angle
							I012[1] = ptocomplex(topolar(Model7MaxPhaseCurr, cang(I012[1])));
						if(ForceBalanced)  // set the negative sequence current
							I012[2] = CZero;
						else
							I012[2] = cdiv(V012[2], with1.Zthev);  // for inverter ZThev is  (Xd' + j0)
					}
					break;
                            // Positive Sequence Contribution to Iterminal
					default:
					CalcVthev_Dyn();  // Update for latest phase angle

                            // Positive Sequence Contribution to Iterminal
					I012[1] = cdiv(csub(V012[1], Vthev), with1.Zthev);  // ZThev is based on Xd'
					I012[2] = cdiv(V012[2], cmplx(0.0, with1.Xdpp));  // machine use Xd"
					break;
				}

                      /*Adjust for WindGen connection*/
				if((Connection == 1) || ForceBalanced)
					I012[0] = CZero;
				else
					I012[0] = cdiv(V012[0], cmplx(0.0, with1.Xdpp));
				SymComp2Phase(&(Iterminal[0]), (pComplexArray) &I012);  // Convert back to phase components

                      // Neutral current
				if(Connection == 0)
					(Iterminal)[Fnconds - 1] = cnegate(cmulreal(I012[0], 3.0));
			}
			break;
			default:
			DoSimpleMsg(Format("Dynamics mode is implemented only for 1- or 3-phase WindGens. WindGen.%s has %d phases.", get_Name().c_str(), Fnphases), 5671);
			SolutionAbort = true;
			break;
		}
		break;
	}
	set_ITerminalUpdated(true, ActorID);

    /*Add it into inj current array*/
	for(stop = Fnconds, i = 1; i <= stop; i++)
	{
		caccum((InjCurrent)[i - 1], cnegate((Iterminal)[i - 1]));
	}

   /*Take Care of any shaft model calcs*/
	if((GenModel == 6) && ShaftModel->Get_Exists())      // auto selects model
           // Compute Mech Power to shaft
	{
		ShaftModel->FCalc(&(Vterminal[0]), &(Iterminal[0]));     // Returns pshaft at least
	}
}


// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


/*Compute Injection Current Only when in harmonics mode*/

/*Assumes spectrum is a voltage source behind subtransient reactance and YPrim has been built*/
/*Vd is the fundamental frequency voltage behind Xd" for phase 1*/

void TWindGenObj::DoHarmonicMode(int ActorID)
{
	int i = 0;
	complex e = {};
	double GenHarmonic = 0.0;
	ComputeVterminal(ActorID);
	/*# with ActiveCircuit[ActorID].Solution do */
	{
		auto with0 = ActiveCircuit[ActorID]->Solution;
		int stop = 0;
		GenHarmonic = with0->get_FFrequency() / GenFundamental;
		e = cmulreal(SpectrumObj->GetMult(GenHarmonic), WindGenVars.VThevHarm); // Get base harmonic magnitude
		RotatePhasorRad(e, GenHarmonic, WindGenVars.ThetaHarm);  // Time shift by fundamental frequency phase shift
		for(stop = Fnphases, i = 1; i <= stop; i++)
		{
			cBuffer[i - 1] = e;
			if(i < Fnphases)
				RotatePhasorDeg(e, GenHarmonic, -120.0);  // Assume 3-phase WindGen
		}
	}

   /*Handle Wye Connection*/
	if(Connection == 0)
		cBuffer[Fnconds - 1] = (Vterminal)[Fnconds - 1];  // assume no neutral injection voltage
		
   /*Inj currents = Yprim (E) */
	YPrim->MVmult(InjCurrent, (pComplexArray) &cBuffer);
}


// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::CalcVTerminalPhase(int ActorID)
{
	int i = 0;
	int j = 0;

/* Establish phase voltages and stick in Vterminal*/
	switch(Connection)
	{
		case 	0:
		{
			/*# with ActiveCircuit[ActorID].Solution do */
			{
				auto with0 = ActiveCircuit[ActorID]->Solution;
				int stop = 0;
				for(stop = Fnphases, i = 1; i <= stop; i++)
				{
					(Vterminal)[i - 1] = with0->VDiff((NodeRef)[i - 1], (NodeRef)[Fnconds - 1], ActorID);
				}
			}
		}
		break;
		case 	1:
		{
			/*# with ActiveCircuit[ActorID].Solution do */
			{
				auto with1 = ActiveCircuit[ActorID]->Solution;
				int stop = 0;
				for(stop = Fnphases, i = 1; i <= stop; i++)
				{
					j = i + 1;
					if(j > Fnconds)
						j = 1;
					(Vterminal)[i - 1] = with1->VDiff((NodeRef)[i - 1], (NodeRef)[j - 1], ActorID);
				}
			}
		}
		break;
		default:
		  ;
		break;
	}
	WindGenSolutionCount = ActiveCircuit[ActorID]->Solution->SolutionCount;
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


/*Put terminal voltages in an array*/

void TWindGenObj::CalcVterminal(int ActorID)
{
	ComputeVterminal(ActorID);
	WindGenSolutionCount = ActiveCircuit[ActorID]->Solution->SolutionCount;
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

// Calculates WindGen current and adds it properly into the injcurrent array
// routines may also compute ITerminal  (ITerminalUpdated flag)

void TWindGenObj::CalcGenModelContribution(int ActorID)
{
	set_ITerminalUpdated(false, ActorID);
	/*# with ActiveCircuit[ActorID], ActiveCircuit[ActorID].Solution do */
	{
		
		auto with1 = ActiveCircuit[ActorID]->Solution;
		if(with1->IsDynamicModel)
			DoDynamicMode(ActorID);
		else
		{
			if(with1->IsHarmonicModel && (with1->get_FFrequency() != ActiveCircuit[ActorID]->Fundamental))
				DoHarmonicMode(ActorID);
			else

           //  compute currents and put into InjTemp array;
			{
				switch(GenModel)
				{
					case 	1:
					DoConstantPQGen(ActorID);
					break;
					case 	2:
					DoConstantZGen(ActorID);
					break;
					case 	3:
					DoPVTypeGen(ActorID);
					break;  // Constant P, |V|
					case 	4:
					DoFixedQGen(ActorID);
					break;
					case 	5:
					DoFixedQZGen(ActorID);
					break;
					case 	6:
					DoUserModel(ActorID);
					break;
					case 	7:
					DoCurrentLimitedPQ(ActorID);
					break;
					default:
					DoConstantPQGen(ActorID);  // for now, until we implement the other models.
					break;
				}
			} /*ELSE*/
		}
	} /*WITH*/

   /*When this is done, ITerminal is up to date*/
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -



// Difference between currents in YPrim and total current

void TWindGenObj::CalcInjCurrentArray(int ActorID)
{

      

// Now Get Injection Currents
	if(GenSwitchOpen)
		ZeroInjCurrent();
	else
		CalcGenModelContribution(ActorID);

/*  We're not going to mess with this logic here -- too complicated: Use an open line in series
    to look at open phase conditions.

  ELSE Begin

   // some terminals not closed  use admittance model for injection
      If OpenWindGenSolutionCount <> ActiveCircuit[ActiveActor].Solution.SolutionCount Then Begin

      // Rebuild the Yprimopencond if a new solution because values may have changed.

        // only reallocate when necessary
        If YPrimOpenCond=nil Then YPrimOpenCond := TcMatrix.CreateMatrix(Yorder)
        ELSE YPrimOpenCond.Clear;
        If YPrimOpenCond.Order <> Yorder Then Begin
           YPrimOpenCond.Free;
           YPrimOpenCond := TcMatrix.CreateMatrix(Yorder);
        End;
        CalcYPrimMatrix(YPrimOpenCond);

        {Now Account for the Open Conductors}
        {For any conductor that is open, zero out row and column}
         With YPrimOpenCond Do Begin
           k := 0;
           FOR i := 1 TO Fnterms Do Begin
             FOR j := 1 TO Fnconds Do Begin
                 If Not Terminals^[i].Conductors^[j].Closed Then Begin
                    ZeroRow(j+k);
                    ZeroCol(j+k);
                    SetElement(j+k, j+k, Cmplx(1.0e-12,0.0));  // In case node gets isolated
                 End;
             End;
             k := k+Fnconds;
           End;
         End;
         OpenWindGenSolutionCount := ActiveCircuit[ActiveActor].Solution.SolutionCount;
         
      End;

      With ActiveCircuit[ActiveActor].Solution Do
      FOR i := 1 TO Yorder Do Begin
          Ref := NodeRef^[i];
          If Ref=0 Then Vterminal^[i] := cZero
          ELSE  Vterminal^[i] := V^[ref];
      End;
      YPrimOpenCond.MVmult(InjTemp, Vterminal);
      For i := 1 to Yorder Do InjTemp^[i] := Cnegate(InjTemp^[i]);
   End;
 */
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


// Compute total Currents

void TWindGenObj::GetTerminalCurrents(pComplexArray Curr, int ActorID)
{
	/*# with ActiveCircuit[ActorID].Solution do */
	{
		auto with0 = ActiveCircuit[ActorID]->Solution;
		if(IterminalSolutionCount[ActorID] != ActiveCircuit[ActorID]->Solution->SolutionCount)     // recalc the contribution
		{
			if(!GenSwitchOpen)
				CalcGenModelContribution(ActorID);  // Adds totals in Iterminal as a side effect
		}
		inherited::GetTerminalCurrents(Curr, ActorID);
	}
	if(DebugTrace)
		WriteTraceRecord("TotalCurrent", ActorID);
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

int TWindGenObj::InjCurrents(int ActorID)
{
	int result = 0;
	/*# with ActiveCircuit[ActorID].Solution do */
	{
		auto with0 = ActiveCircuit[ActorID]->Solution;
		if(with0->LoadsNeedUpdating)
			SetNominalGeneration(ActorID); // Set the nominal kW, etc for the type of solution being done
		CalcInjCurrentArray(ActorID);          // Difference between currents in YPrim and total terminal current
		if(DebugTrace)
			WriteTraceRecord("Injection", ActorID);

       // Add into System Injection Current Array
		result = inherited::InjCurrents(ActorID);
	}
	return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -


// Gives the currents for the last solution performed

// Do not call SetNominalLoad, as that may change the load values

void TWindGenObj::GetInjCurrents(pComplexArray Curr, int ActorID)
{
	int i = 0;
	CalcInjCurrentArray(ActorID);  // Difference between currents in YPrim and total current
	try

   // Copy into buffer array
	{
		int stop = 0;
		for(stop = Yorder, i = 1; i <= stop; i++)
		{
			(Curr)[i - 1] = (InjCurrent)[i - 1];
		}
	}
	catch (std::exception &e)
	{
		DoErrorMsg(String("WindGen Object: \"") + get_Name() +"\" in GetInjCurrents function.", (std::string) e.what(), "Current buffer not big enough.", 568);
	}
}
//= = =  = = = = = = = = = = = = = = = = = = = = = = = = = = = =


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TWindGenObj::ResetRegisters()
{
	int i = 0;
	int stop = 0;
	for(stop = NumGenRegisters, i = 1; i <= stop; i++)
	{
		Registers[i - 1] = 0.0;
	}
	for(stop = NumGenRegisters, i = 1; i <= stop; i++)
	{
		Derivatives[i - 1] = 0.0;
	}
	FirstSampleAfterReset = true;  // initialize for trapezoidal integration
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TWindGenObj::Integrate(int reg, double Deriv, double Interval, int ActorID)
{
	if(ActiveCircuit[ActorID]->TrapezoidalIntegration)
        /*Trapezoidal Rule Integration*/
	{
		if(!FirstSampleAfterReset)
			Registers[reg] = Registers[reg] + 0.5 * Interval * (Deriv + Derivatives[reg]);
	}
	else
   /*Plain Euler integration*/
	Registers[reg] = Registers[reg] + Interval * Deriv;
	Derivatives[reg] = Deriv;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Update Energy from metered zone

void TWindGenObj::TakeSample(int ActorID)
{
	complex s = {};
	double Smag = 0.0;
	double HourValue = 0.0;

// Compute energy in WindGen branch
	if(Get_Enabled())
	{
		if(GenON)
		{
			s = cmplx(Get_PresentkW(), Get_Presentkvar());
			Smag = cabs(s);
			HourValue = 1.0;
		}
		else
		{
			s = CZero;
			Smag = 0.0;
			HourValue = 0.0;
		}
		if(GenON || ActiveCircuit[ActorID]->TrapezoidalIntegration)
			/*# with ActiveCircuit[ActorID].Solution do */
			{
				auto with0 = ActiveCircuit[ActorID]->Solution;
      /*Make sure we always integrate for Trapezoidal case
       Don't need to for Gen Off and normal integration*/
				if(ActiveCircuit[ActorID]->PositiveSequence)
				{
					s = cmulreal(s, 3.0);
					Smag = 3.0 * Smag;
				}
				Integrate(Reg_kWh, s.re, with0->IntervalHrs, ActorID);   // Accumulate the power
				Integrate(Reg_kvarh, s.im, with0->IntervalHrs, ActorID);
				SetDragHandRegister(Reg_MaxkW, Abs((int) s.re));
				SetDragHandRegister(Reg_MaxkVA, Smag);
				Integrate(Reg_Hours, HourValue, with0->IntervalHrs, ActorID);  // Accumulate Hours in operation
				Integrate(Reg_Price, s.re * ActiveCircuit[ActorID]->PriceSignal * 0.001, with0->IntervalHrs, ActorID);  // Accumulate Hours in operation
				FirstSampleAfterReset = false;
			}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

double TWindGenObj::Get_PresentkW()
{
	double result = 0.0;
	result = WindGenVars.Pnominalperphase * 0.001 * Fnphases;
	return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

double TWindGenObj::Get_PresentkV()
{
	double result = 0.0;
	result = WindGenVars.kVWindGenBase;
	return result;
}

double TWindGenObj::Get_Presentkvar()
{
	double result = 0.0;
	result = WindGenVars.Qnominalperphase * 0.001 * Fnphases;
	return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::InitDQDVCalc()
{
	DQDV = 0.0;
	WindGenVars.Qnominalperphase = 0.5 * (varMax + varMin);   // avg of the limits
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

/*Bump up vars by 10% of range for next calc*/

void TWindGenObj::BumpUpQ()
{
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		with0.Qnominalperphase = with0.Qnominalperphase + 0.1 * (varMax - varMin);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::RememberQV(int ActorID)
{
	int i = 0;
	int stop = 0;
	var_Remembered = WindGenVars.Qnominalperphase;
	CalcVterminal(ActorID);
	V_Avg = 0.0;
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		V_Avg = V_Avg + cabs((Vterminal)[i - 1]);
	}
	V_Avg = V_Avg / Fnphases;
	V_Remembered = V_Avg;
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::CalcDQDV(int ActorID)
{
	double VDiff = 0.0;
	int i = 0;
	int stop = 0;
	CalcVterminal(ActorID);
	V_Avg = 0.0;
	for(stop = Fnphases, i = 1; i <= stop; i++)
	{
		V_Avg = V_Avg + cabs((Vterminal)[i - 1]);
	}
	V_Avg = V_Avg / Fnphases;
	VDiff = V_Avg - V_Remembered;
	if(VDiff != 0.0)
		DQDV = (WindGenVars.Qnominalperphase - var_Remembered) / VDiff;
	else
		DQDV = 0.0;  // Something strange has occured
                       // this will force a de facto P,Q model
	DQDVSaved = DQDV;  //Save for next time  Allows WindGen to be enabled/disabled during simulation
}

// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::ResetStartPoint()
{
	WindGenVars.Qnominalperphase = 1000.0 * kvarBase / Fnphases;
}


// - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - -

void TWindGenObj::DumpProperties(TTextRec& f, bool Complete)
{
	int i = 0;
	int Idx = 0;
	inherited::DumpProperties(f, Complete);
	{ Write(f,"!DQDV="); WriteLn(f, DQDV, 10, 2); }
	/*# with ParentClass do */
	{
		auto with0 = ParentClass;
		int stop = 0;
		for(stop = with0->NumProperties, i = 1; i <= stop; i++)
		{
			Idx = (with0->PropertyIdxMap)[i - 1];
			switch(Idx)
			{
				case 	34: case 36:
				{ Write(f,"~ "); Write(f, (with0->PropertyName)[i - 1]); Write(f,"=("); Write(f, Get_PropertyValue(Idx)); WriteLn(f, L')'); }
				break;
				case 	44:  // This one has no variable associated, not needed
				{ Write(f,"~ "); Write(f, (with0->PropertyName)[i - 1]); WriteLn(f,"=False"); }
				break;
				default:
				{ Write(f,"~ "); Write(f, (with0->PropertyName)[i - 1]); Write(f, L'='); WriteLn(f, Get_PropertyValue(Idx)); }
				break;
			}
		}
	}
	WriteLn(f);
}

void TWindGenObj::InitHarmonics(int ActorID)
{
	complex e = {};
	complex Va = {};
	Set_YprimInvalid(ActorID,true);  // Force rebuild of YPrims
	GenFundamental = ActiveCircuit[ActorID]->Solution->get_FFrequency();  // Whatever the frequency is when we enter here.
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		Yeq = cinv(cmplx(0.0, with0.Xdpp));      // used for current calcs  Always L-N

         /*Compute reference Thevinen voltage from phase 1 current*/
		if(GenON)
		{
			ComputeIterminal(ActorID);  // Get present value of current
			/*# with ActiveCircuit[ActorID].Solution do */
			{
				auto with1 = ActiveCircuit[ActorID]->Solution;
				switch(Connection)
				{
					case 	0: /*wye - neutral is explicit*/
					{
						if(!ADiakoptics || (ActorID == 1))
							Va = csub(with1->NodeV[(NodeRef)[1 - 1]], with1->NodeV[(NodeRef)[Fnconds - 1]]);
						else
							Va = csub(with1->VoltInActor1((NodeRef)[1 - 1]), with1->VoltInActor1((NodeRef)[Fnconds - 1]));
					}
					break;  /*delta -- assume neutral is at zero*/
					case 	1:
					{
						if(!ADiakoptics || (ActorID == 1))
							Va = with1->NodeV[(NodeRef)[1 - 1]];
						else
							Va = with1->VoltInActor1((NodeRef)[1 - 1]);
					}
					break;
					default:
					  ;
					break;
				}
			}
			e = csub(Va, cmul((Iterminal)[1 - 1], cmplx(0.0, with0.Xdpp)));
			with0.VThevHarm = cabs(e);   // establish base mag and angle
			with0.ThetaHarm = cang(e);
		}
		else
		{
			with0.VThevHarm = 0.0;
			with0.ThetaHarm = 0.0;
		}
	}
}

void TWindGenObj::InitPropertyValues(int ArrayOffset)
{
	Set_PropertyValue(1,"3");     //'phases';
	Set_PropertyValue(2,GetBus(1));         //'bus1';
	Set_PropertyValue(3,"12.47");
	Set_PropertyValue(4,"100");
	Set_PropertyValue(5,".80");
	Set_PropertyValue(6,"1");
	Set_PropertyValue(7,"");
	Set_PropertyValue(8,"");
	Set_PropertyValue(9,"");
	Set_PropertyValue(10,"Default");
	Set_PropertyValue(11,"0.0");
	Set_PropertyValue(12,"wye");
	Set_PropertyValue(13,"60");
	Set_PropertyValue(14,"0"); // 'rneut'; // if entered -, assume open
	Set_PropertyValue(15,"0");  //'xneut';
	Set_PropertyValue(16,"variable"); //'status'  fixed or variable
	Set_PropertyValue(17,"1"); //'class'
	Set_PropertyValue(18,"1.0");
	Set_PropertyValue(19,Str_Real(kvarMax, 3));
	Set_PropertyValue(20,Str_Real(kvarMin, 3));
	Set_PropertyValue(21,"0.1");
	Set_PropertyValue(22,"no");
	Set_PropertyValue(23,"0.90");
	Set_PropertyValue(24,"1.10");
	Set_PropertyValue(25,"No");
	Set_PropertyValue(26,Format("%-g", WindGenVars.kVArating));
	Set_PropertyValue(27,Format("%-g", WindGenVars.kVArating * 0.001));
	Set_PropertyValue(28,Format("%-g", WindGenVars.puXd));
	Set_PropertyValue(29,Format("%-g", WindGenVars.puXdp));
	Set_PropertyValue(30,Format("%-g", WindGenVars.puXdpp));
	Set_PropertyValue(31,Format("%-g", WindGenVars.Hmass));
	Set_PropertyValue(32,Format("%-g", WindGenVars.Dpu));
	Set_PropertyValue(33,"");
	Set_PropertyValue(34,"");
	Set_PropertyValue(35,"");
	Set_PropertyValue(36,"");
	Set_PropertyValue(37,"0");
	Set_PropertyValue(38,"No");
	Set_PropertyValue(39,"20");
	Set_PropertyValue(40,"No");
	Set_PropertyValue(41,"0.0");
	Set_PropertyValue(42,"100.0");
	Set_PropertyValue(43,"20.0");
	Set_PropertyValue(44,"No");
	inherited::InitPropertyValues(NumPropsThisClass);
}

void TWindGenObj::InitStateVars(int ActorID)
{

    /*VNeut,*/
	int i = 0;
	complex V012[4] = { CZero, CZero, CZero, CZero, },
			I012[4] = { CZero, CZero, CZero, CZero, },
			Vabc[4] = { CZero, CZero, CZero, CZero, };
	Set_YprimInvalid(ActorID,true);  // Force rebuild of YPrims
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		switch(GenModel)
		{
			case 	7:
			with0.Zthev = cmplx(with0.Xdp, 0.0);
			break; // use Xd' as an equivalent R for the inverter
			default:
			with0.Zthev = cmplx(with0.Xdp / with0.XRdp, with0.Xdp);
			break;
		}
		Yeq = cinv(with0.Zthev);

     /*Compute nominal Positive sequence voltage behind transient reactance*/
		if(GenON)
			/*# with ActiveCircuit[ActorID].Solution do */
			{
				auto with1 = ActiveCircuit[ActorID]->Solution;
				ComputeIterminal(ActorID);
				switch(Fnphases)
				{
					case 	1:
					{
						if(!ADiakoptics || (ActorID == 1))
							Edp = csub(csub(with1->NodeV[(NodeRef)[1 - 1]], with1->NodeV[(NodeRef)[2 - 1]]), cmul((Iterminal)[1 - 1], with0.Zthev));
						else
							Edp = csub(csub(with1->VoltInActor1((NodeRef)[1 - 1]), with1->VoltInActor1((NodeRef)[2 - 1])), cmul((Iterminal)[1 - 1], with0.Zthev));
						with0.VthevMag = cabs(Edp);
					}
					break;
                 // Calculate Edp based on Pos Seq only
					case 	3:
					{
						int stop = 0;
						Phase2SymComp(&(Iterminal[0]), &I012[0]);
                     // Voltage behind Xdp  (transient reactance), volts
						for(stop = Fnphases, i = 1; i <= stop; i++)
						{
							if(!ADiakoptics || (ActorID == 1))
								Vabc[i] = with1->NodeV[(NodeRef)[i - 1]];
							else
								Vabc[i] = with1->VoltInActor1((NodeRef)[i - 1]);
						}   // Wye Voltage
						Phase2SymComp(&Vabc[1], &V012[0]);
						Edp = csub(V012[1], cmul(I012[1], with0.Zthev));    // Pos sequence
						with0.VthevMag = cabs(Edp);
					}
					break;
					default:
					DoSimpleMsg(Format(("Dynamics mode is implemented only for 1- or 3-phase WindGens. WindGen." + with1->get_Name()
	           +" has %d phases.").c_str(), Fnphases), 5672);
					SolutionAbort = true;
					break;
				}


         // Shaft variables
         // Theta is angle on Vthev[1] relative to system reference
         //Theta  := Cang(Vthev^[1]);   // Assume source at 0
				with0.Theta = cang(Edp);
				if(GenModel == 7)
					Model7LastAngle = with0.Theta;
				with0.dTheta = 0.0;
				with0.w0 = TwoPi * ActiveCircuit[ActorID]->Solution->get_FFrequency();
         // recalc Mmass and D in case the frequency has changed
				/*# with WindGenVars do */
				{
					auto& with2 = WindGenVars;
					WindGenVars.Mmass = 2.0 * WindGenVars.Hmass * WindGenVars.kVArating * 1000.0 / (with2.w0);   // M = W-sec
					with2.D = with2.Dpu * with2.kVArating * 1000.0 / (with2.w0);
				}
				with0.Pshaft = -Get_Power(1, ActorID).re; // Initialize Pshaft to present power Output
				with0.Speed = 0.0;    // relative to synch speed
				with0.dSpeed = 0.0;

         // Init User-written models
         //Ncond:Integer; V, I:pComplexArray; const X,Pshaft,Theta,Speed,dt,time:Double
				/*# with ActiveCircuit[ActorID].Solution do */
				{
					auto with3 = ActiveCircuit[ActorID]->Solution;
					if(GenModel == 6)
					{
						if(UserModel->Get_Exists())
							UserModel->FInit(&(Vterminal[0]), &(Iterminal[0]));
						if(ShaftModel->Get_Exists())
							ShaftModel->FInit(&(Vterminal[0]), &(Iterminal[0]));
					}
				}
			}
		else
		{
			Vthev = CZero;
			with0.Theta = 0.0;
			with0.dTheta = 0.0;
			with0.w0 = 0;
			with0.Speed = 0.0;
			with0.dSpeed = 0.0;
		}
	}  /*With*/
}

void TWindGenObj::IntegrateStates(int ActorID)
{
	complex TracePower = {};
   // Compute Derivatives and then integrate
	ComputeIterminal(ActorID);

// Check for user-written exciter model.
    //Function(V, I:pComplexArray; const Pshaft,Theta,Speed,dt,time:Double)
	/*# with ActiveCircuit[ActorID].Solution, WindGenVars do */
	{
		auto with0 = ActiveCircuit[ActorID]->Solution;
		auto& with1 = WindGenVars;
		/*# with DynaVars do */
		{
			auto& with2 = with0->DynaVars;
			if(with2.IterationFlag == 0) /*Get_First() iteration of new time step*/
			{
				with1.ThetaHistory = with1.Theta + 0.5 * with2.h * with1.dTheta;
				with1.SpeedHistory = with1.Speed + 0.5 * with2.h * with1.dSpeed;
			}
		}

      // Compute shaft dynamics
		TracePower = TerminalPowerIn(&(Vterminal[0]), &(Iterminal[0]), Fnphases);
		with1.dSpeed = (with1.Pshaft + TracePower.re - with1.D * with1.Speed) / with1.Mmass;
//      dSpeed := (Torque + TerminalPowerIn(Vtemp,Itemp,FnPhases).re/Speed) / (Mmass);
		with1.dTheta = with1.Speed;

     // Trapezoidal method
		/*# with DynaVars do */
		{
			auto& with3 = with0->DynaVars;
			with1.Speed = with1.SpeedHistory + 0.5 * with3.h * with1.dSpeed;
			with1.Theta = with1.ThetaHistory + 0.5 * with3.h * with1.dTheta;
		}

      // Write Dynamics Trace Record
		if(DebugTrace)
		{
			Append(Tracefile);
			IOResultToException();
			Write(Tracefile, Format("t=%-.5g ", with0->DynaVars.T));
			Write(Tracefile, Format(" Flag=%d ", with0->DynaVars.IterationFlag));
			Write(Tracefile, Format(" Speed=%-.5g ", with1.Speed));
			Write(Tracefile, Format(" dSpeed=%-.5g ", with1.dSpeed));
			Write(Tracefile, Format(" Pshaft=%-.5g ", with1.Pshaft));
			Write(Tracefile, Format(" P=%-.5g Q= %-.5g", TracePower.re, TracePower.im));
			Write(Tracefile, Format(" M=%-.5g ", with1.Mmass));
			WriteLn(Tracefile);
			CloseFile(Tracefile);
		}
		if(GenModel == 6)
		{
			if(UserModel->Get_Exists())
				UserModel->Integrate();
			if(ShaftModel->Get_Exists())
				ShaftModel->Integrate();
		}
	}
}
/*Return variables one at a time*/

double TWindGenObj::Get_Variable(int i)
{
	double result = 0.0;
	int n = 0;
	int k = 0;
	n = 0;
	result = -9999.99;  // error return value
	if(i < 1)
		return result;  // Someone goofed
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		switch(i)
		{
			case 	1:
			result = (with0.w0 + with0.Speed) / TwoPi;
			break;  // Frequency, Hz
			case 	2:
			result = (with0.Theta) * RadiansToDegrees;
			break;  // Report in Deg
			case 	3:
			result = cabs(Vthev) / VBase;
			break;      // Report in pu
			case 	4:
			result = with0.Pshaft;
			break;
			case 	5:
			result = with0.dSpeed * RadiansToDegrees;
			break; // Report in Deg      57.29577951
			case 	6:
			result = with0.dTheta;
			break;
			default:
			if(UserModel->Get_Exists())
			{
				n = UserModel->FNumVars();
				k = (i - NumGenVariables);
				if(k <= n)
				{
					result = UserModel->FGetVariable(k);
					return result;
				}
			}

           /*If we get here, must be in the Shaft Model if anywhere*/
			if(ShaftModel->Get_Exists())
			{
				k = i - (NumGenVariables + n);
				if(k > 0)
					result = ShaftModel->FGetVariable(k);
			}
			break;
		}
	}
	return result;
}

void TWindGenObj::Set_Variable(int i, double Value)
{
	int n = 0;
	int k = 0;
	n = 0;
	if(i < 1)
		return;  // Someone goofed
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		switch(i)
		{
			case 	1:
			with0.Speed = (Value - with0.w0) * TwoPi;
			break;
			case 	2:
			with0.Theta = Value / RadiansToDegrees;
			break; // deg to rad
			case 	3:
			;
			break;// meaningless to set Vd := Value * vbase; // pu to volts
			case 	4:
			with0.Pshaft = Value;
			break;
			case 	5:
			with0.dSpeed = Value / RadiansToDegrees;
			break;
			case 	6:
			with0.dTheta = Value;
			break;
			default:
			if(UserModel->Get_Exists())
			{
				n = UserModel->FNumVars();
				k = (i - NumGenVariables);
				if(k <= n)
				{
					UserModel->FSetVariable(k, Value);
					return;
				}
			}
         // If we get here, must be in the shaft model
			if(ShaftModel->Get_Exists())
			{
				k = (i - (NumGenVariables + n));
				if(k > 0)
					ShaftModel->FSetVariable(k, Value);
			}
			break;
		}
	}
}

void TWindGenObj::GetAllVariables(pDoubleArray States)
{
	int i = 0;
	int n = 0;
	int stop = 0;
	n = 0;
	for(stop = NumGenVariables, i = 1; i <= stop; i++)
	{
		(States)[i - 1] = Get_Variable(i);
	}
	if(UserModel->Get_Exists())
	{
		n = UserModel->FNumVars();
		UserModel->FGetAllVars((pDoubleArray)&(States)[NumGenVariables + 1 - 1]);
	}
	if(ShaftModel->Get_Exists())
	{
		ShaftModel->FGetAllVars((pDoubleArray)&(States)[NumGenVariables + 1 + n - 1]);
	}
}

int TWindGenObj::NumVariables()
{
	int result = 0;
	result = NumGenVariables;
	if(UserModel->Get_Exists())
		result = result + UserModel->FNumVars();
	if(ShaftModel->Get_Exists())
		result = result + ShaftModel->FNumVars();
	return result;
}

//---------------------------------------------------------------------------

bool TWindGenObj::get_FForcedON()
{
	return FForcedON;
}

//---------------------------------------------------------------------------

void TWindGenObj::set_FForcedON(bool myState)
{
	FForcedON = myState;
}

//---------------------------------------------------------------------------

double TWindGenObj::get_PFNominal()
{
	return PFNominal;
}

//---------------------------------------------------------------------------

String TWindGenObj::VariableName(int i)
{
	String result;
	const int BuffSize = 255;
	int n = 0;
	int I2 = 0;
	AnsiChar Buff[256/*# range 0..BuffSize*/];
	PAnsiChar PName = nullptr;
	n = 0;
	if(i < 1)
		return result;  // Someone goofed
	switch(i)
	{
		case 	1:
		result ="Frequency";
		break;
		case 	2:
		result ="Theta (Deg)";
		break;
		case 	3:
		result ="Vd";
		break;
		case 	4:
		result ="PShaft";
		break;
		case 	5:
		result ="dSpeed (Deg/sec)";
		break;
		case 	6:
		result ="dTheta (Deg)";
		break;
		default:
		if(UserModel->Get_Exists())  // Checks for existence and Selects
		{
			PName = (PAnsiChar) &Buff;
			n = UserModel->FNumVars();
			I2 = i - NumGenVariables;
			if(I2 <= n)
                 // DLL functions require AnsiString (AnsiString) type
			{
				UserModel->FGetVarName(I2, PName, (unsigned int) BuffSize);
				result = PName;
				return result;
			}
		}
		if(ShaftModel->Get_Exists())
		{
			PName = (PAnsiChar) &Buff;
			I2 = i - NumGenVariables - n;
			if(I2 > 0)
				UserModel->FGetVarName(I2, PName, (unsigned int) BuffSize);
			result = PName;
		}
		break;
	}
	return result;
}

String TWindGenObj::GetPropertyValue(int Index)
{
	String result;
	result ="";
	switch(Index)
	{
		case 	3:
		result = Format("%.6g", WindGenVars.kVWindGenBase);
		break;
		case 	4:
		result = Format("%.6g", kWBase);
		break;
		case 	5:
		result = Format("%.6g", PFNominal);
		break;
		case 	7:
		result = YearlyShape;
		break;
		case 	8:
		result = DailyDispShape;
		break;
		case 	9:
		result = DutyShape;
		break;
		case 	13:
		result = Format("%.6g", kvarBase);
		break;
		case 	19:
		result = Format("%.6g", kvarMax);
		break;
		case 	20:
		result = Format("%.6g", kvarMin);
		break;
		case 	26:
		result = Format("%.6g", WindGenVars.kVArating);
		break;
		case 	27:
		result = Format("%.6g", WindGenVars.kVArating * 0.001);
		break;
		case 	34:
		 case 36:
		{
			result = String("(") + inherited::GetPropertyValue(Index) +")";
		}
		break;
		case 	37:
		result = Format("%.6g", DutyStart);
		break;
		case 	38:
		if(ForceBalanced)
			result ="Yes";
		else
			result ="No";
		break;
		default:
		result = inherited::GetPropertyValue(Index);
		break;
	}
	return result;
}

void TWindGenObj::MakePosSequence(int ActorID)
{
	String s;
	double V = 0.0;
	s ="Phases=1 conn=wye";

  // Make sure voltage is line-neutral
	if((Fnphases > 1) || (Connection != 0))
		V = WindGenVars.kVWindGenBase / SQRT3;
	else
		V = WindGenVars.kVWindGenBase;
	s = s + Format(" kV=%-.5g", V);

  // Divide the load by no. phases
	if(Fnphases > 1)
	{
		s = s + Format(" kW=%-.5g  PF=%-.5g", kWBase / Fnphases, PFNominal);
		if((PrpSequence[19 - 1] != 0) || (PrpSequence[20 - 1] != 0 ))
			s = s
	           + Format(" maxkvar=%-.5g  minkvar=%-.5g", kvarMax / Fnphases, kvarMin / Fnphases);
		if((PrpSequence)[26 - 1] > 0)
			s = s + Format(" kva=%-.5g  ", WindGenVars.kVArating / Fnphases);
		if((PrpSequence)[27 - 1] > 0)
			s = s + Format(" MVA=%-.5g  ", WindGenVars.kVArating / 1000.0 / Fnphases);
	}
	Parser[ActorID]->SetCmdString(s);
	Edit(ActorID);
	inherited::MakePosSequence(ActorID);
}

void TWindGenObj::Set_ConductorClosed(int Index, int ActorID, bool Value)
{
	inherited::Set_ConductorClosed(Index, ActorID, Value);

 // Just turn WindGen on or off;
	if(Value)
		GenSwitchOpen = false;
	else
		GenSwitchOpen = true;
}

void TWindGenObj::Set_PowerFactor(double Value)
{
	PFNominal = Value;
	SyncUpPowerQuantities();
}

void TWindGenObj::Set_PresentkV(double Value)
{
	/*# with WindGenVars do */
	{
		auto& with0 = WindGenVars;
		with0.kVWindGenBase = Value;
		switch(Fnphases)
		{
			case 	2:
			 case 3:
			VBase = with0.kVWindGenBase * InvSQRT3x1000;
			break;
			default:
			VBase = with0.kVWindGenBase * 1000.0;
			break;
		}
	}
}

void TWindGenObj::Set_Presentkvar(double Value)
{
	double kVA_Gen = 0.0;
	kvarBase = Value;
	WindGenVars.Qnominalperphase = 1000.0 * kvarBase / Fnphases; // init to something reasonable
	kVA_Gen = sqrt(Sqr(kWBase) + Sqr(kvarBase));
	if(kVA_Gen != 0.0)
		PFNominal = kWBase / kVA_Gen;
	else
		PFNominal = 1.0;
	if((kWBase * kvarBase) < 0.0)
		PFNominal = -PFNominal;
	kvarMax = 2.0 * kvarBase;
	kvarMin = -kvarMax;
}

void TWindGenObj::Set_PresentkW(double Value)
{
	kWBase = Value;
	SyncUpPowerQuantities();
}

void TWindGenObj::SyncUpPowerQuantities()
{


   // keep kvar nominal up to date with kW and PF
	if(PFNominal != 0.0)
	{
		kvarBase = kWBase * sqrt(1.0L / Sqr(PFNominal) - 1.0L);
		WindGenVars.Qnominalperphase = 1000.0 * kvarBase / Fnphases;
		kvarMax = 2.0 * kvarBase;
		kvarMin = -kvarMax;
		if(PFNominal < 0.0)
			kvarBase = -kvarBase;
		if(kVANotSet)
			WindGenVars.kVArating = kWBase * 1.2;
	}
}

void TWindGenObj::SetDragHandRegister(int reg, double Value)
{
	if(Value > Registers[reg])
		Registers[reg] = Value;
}

void TWindGenObj::SetkWkvar(double PkW, double Qkvar)
{
	kWBase = PkW;
	Set_Presentkvar(Qkvar);
}

void TWindGenObj::CalcVthev_Dyn()
{
	if(GenSwitchOpen)
		WindGenVars.VthevMag = 0.0;
	Vthev = pclx(WindGenVars.VthevMag, WindGenVars.Theta);
}
/*Adjust VThev to be in phase with V, if possible*/
/*
 If the voltage magnitude drops below 15% or so, the accuracy of determining the
 phase angle gets flaky. This algorithm approximates the action of a PLL that will
 hold the last phase angle until the voltage recovers.
*/

void TWindGenObj::CalcVthev_Dyn_Mod7(const complex& V)
{
	double Model7angle = 0.0;
	if(GenSwitchOpen)
		WindGenVars.VthevMag = 0.0;
   /*
      For Phases=1, Vbase is voltage across the terminals.
      Else it is LN voltage.
   */
	if(cabs(V) > 0.2 * VBase)
		Model7angle = cang(V);
	else
		Model7angle = Model7LastAngle;
	Vthev = pclx(WindGenVars.VthevMag, Model7angle);
	Model7LastAngle = Model7angle;
}


void WindGen_initialization()
{
	CDoubleOne = cmplx(1.0, 1.0);
//   TWOPI3     := twopi/3.0;
}

		class 		WindGen_unit
		{
		public:
		WindGen_unit()
		{
			//AssertSystemInitialization();
			WindGen_initialization();
		}
		};
		WindGen_unit _WindGen_unit;

}  // namespace WindGen




