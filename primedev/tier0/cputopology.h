#pragma once

class ICpuTopology;

//---------------------------------------------------------------------------------
// Name: CpuToplogy
// Desc: This class constructs a supported cpu topology implementation object on
//       initialization and forwards calls to it.  This is the Abstraction class
//       in the traditional Bridge Pattern.
//---------------------------------------------------------------------------------
class CpuTopology
{
  public:
	CpuTopology(BOOL bForceCpuid = FALSE);
	~CpuTopology();

	BOOL IsDefaultImpl() const;
	DWORD NumberOfProcessCores() const;
	DWORD NumberOfSystemCores() const;
	DWORD_PTR CoreAffinityMask(DWORD coreIdx) const;

	void ForceCpuid(BOOL bForce);

  private:
	void Destroy_();

	ICpuTopology* m_pImpl;
};
