#pragma once

class vgui_BaseRichText;

class CHudChat
{
  public:
	static CHudChat** allHuds;

	char unknown1[720];

	Color m_sameTeamColor;
	Color m_enemyTeamColor;
	Color m_mainTextColor;
	Color m_networkNameColor;

	char unknown2[12];

	int m_unknownContext;

	char unknown3[8];

	vgui_BaseRichText* m_richText;

	CHudChat* next;
	CHudChat* previous;
};
