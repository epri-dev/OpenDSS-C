
friend void DoBreakLoops(CktTree::TCktTree*& BranchList);
friend void DoMergeParallelLines(CktTree::TCktTree*& BranchList);
friend void DoReduceDangling(CktTree::TCktTree*& BranchList);
friend void DoReduceDefault(CktTree::TCktTree*& BranchList);
friend void DoReduceShortLines(CktTree::TCktTree*& BranchList);
friend void DoReduceSwitches(CktTree::TCktTree*& BranchList);
friend void DoRemoveAll_1ph_Laterals(CktTree::TCktTree*& BranchList);
friend void DoRemoveBranches(CktTree::TCktTree*& BranchList, PDELement::TPDElement* FirstPDElement, bool KeepLoad, const String EditStr);
friend bool IsShortLine(CktElement::TDSSCktElement* const Elem);




