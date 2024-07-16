


friend class TCktTree;
friend class TCktTreeNode;
friend class TZoneEndsList;
friend void BuildActiveBusAdjacencyLists(TAdjArray* lstPD, TAdjArray* lstPC, int ActorID);
friend void FreeAndNilBusAdjacencyLists(TAdjArray& lstPD, TAdjArray& lstPC);
TCktTree* GetIsolatedSubArea(CktElement::TDSSCktElement* StartElement, bool Analyze = false);
friend void FindAllChildBranches(TList* adjLst, int BusNum, TCktTree* BranchList, bool Analyze, CktElement::TDSSCktElement* ActiveBranch);
friend void GetPCElementsConnectedToBus(TList* adjLst, TCktTree* BranchList, bool Analyze);
friend void GetShuntPDElementsConnectedToBus(TList* adjLst, TCktTree* BranchList, bool Analyze);
friend void GetSourcesConnectedToBus(int BusNum, TCktTree* BranchList, bool Analyze);



