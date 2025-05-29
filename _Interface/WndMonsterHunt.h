#pragma once

#ifdef _MONSTER_HUNTING
static constexpr std::uint8_t MAX_DRAWING = 6;

class CWndMonsterHunt : public CWndNeuz
{
	struct drawedData {
		CModelObject* m_pModelObject = nullptr;
		std::uint32_t m_nMonsterID = 0;
		LPDIRECT3DVERTEXBUFFER9 m_pVBGauge = nullptr;
	};

	LPWNDCTRL m_lpBackGround;

	CWndScrollBar		m_wndScrollBar;
	int32_t				m_nSelect;
	int32_t				m_nMax;


	CTexture m_texBorder;

	std::array<LPDIRECT3DVERTEXBUFFER9, 5> m_ArrayObjectiveBuffer;

	std::array<drawedData, MAX_DRAWING> m_ArrayDrawedData;

	BOOL m_bVBGauge;

	CTexture m_GaugeTexture;
	CTexture m_EmptyGaugeTexture;

	std::uint8_t m_nDrawCount; 

public:
	CWndMonsterHunt();

	void LoadMonsterModel(std::uint8_t nIndex, std::uint32_t nMonsterId);

	void SetDescription(std::int32_t nHuntIndex);
	virtual void OnDraw(C2DRender* p2DRender) override;
	virtual	void OnInitialUpdate() override;
	virtual void SetWndRect(CRect rectWnd, BOOL bOnSize) override;
	void OnLButtonDown(UINT nFlags, CPoint point) override;

	void DrawGaugeProgress(C2DRender* p2DRender, CPoint pt, std::uint16_t nHunt);
	void DrawSelectedHuntProgress(C2DRender* p2DRender);
	void DrawMonsterModel(C2DRender* p2DRender);

	virtual BOOL OnChildNotify(UINT message, UINT nID, LRESULT* pLResult) override;
	virtual BOOL Process() override;
	virtual BOOL Initialize(CWndBase* pWndParent = NULL, DWORD dwWndId = 0) override;

	virtual BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) override;
	void UpdateScroll();
	int32_t	GetSelectIndex(const CPoint& point);

	virtual HRESULT RestoreDeviceObjects() override;
	virtual HRESULT InvalidateDeviceObjects() override;
	virtual HRESULT DeleteDeviceObjects() override;

};
#endif