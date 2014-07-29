//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#


#include <display/QtPlotter/QtDrawColors.qo.h>

#include <graphics/X11/X_enter.h>
#include <QColor>
#include <QStringList>
#include <graphics/X11/X_exit.h>

namespace casa {

	static const struct DrowColors {
		const char *name;
		QColor  value;
	} rgbTbl[] = {
		{"Red", QColor(255, 0, 0)},
		{"Blue", QColor(0, 0, 255)},
		{"Green", QColor(0, 255, 0)},
		{"Magenta", QColor(255, 0, 255)},
		{"Orangewheel", QColor(255, 127, 0)},
		{"Pink", QColor(255, 192, 203)},
		{"Yellow", QColor(255, 255, 0)},
		{"Cyan", QColor(0, 255, 255)},
		{"Grey", QColor(128, 128, 128)},
		{"Black", QColor(0, 0, 0)},
		{"AliceBlue", QColor(240, 248, 255)},
		{"Alizarin", QColor(227, 38, 54)},
		{"Amaranth", QColor(229, 43, 80)},
		{"Amber", QColor(255, 191, 0)},
		{"Amethyst", QColor(153, 102, 204)},
		{"Apricot", QColor(251, 206, 177)},
		{"Aqua", QColor(0, 255, 255)},
		{"Aquamarine", QColor(127, 255, 212)},
		{"Asparagus", QColor(123, 160, 91)},
		{"Auburn", QColor(113, 47, 44)},
		{"Azure", QColor(0, 127, 255)},
		{"Babyblue", QColor(111, 255, 255)},
		{"Beige", QColor(245, 245, 220)},
		{"Bistre", QColor(61, 43, 31)},
		{"BlueGreen", QColor(0, 223, 223)},
		{"BlueViolet", QColor(138, 43, 226)},
		{"BondiBlue", QColor(0, 149, 182)},
		{"Brass", QColor(181, 166, 66)},
		{"BrightGreen", QColor(102, 255, 0)},
		{"BrightTurquoise", QColor(8, 232, 222)},
		{"BrilliantRose", QColor(255, 85, 163)},
		{"Bronze", QColor(205, 127, 50)},
		{"Brown", QColor(150, 75, 0)},
		{"Buff ", QColor(240, 220, 130)},
		{"Burgundy", QColor(128, 0, 32)},
		{"BurntOrange", QColor(204, 85, 0)},
		{"BurntSienna", QColor(233, 116, 81)},
		{"BurntUmber", QColor(138, 51, 36)},
		{"CamouflageGreen", QColor(120, 134, 107)},
		{"CaputMortuum", QColor(89, 39, 32)},
		{"Cardinal", QColor(196, 30, 58)},
		{"Carmine", QColor(150, 0, 24)},
		{"CarnationPink", QColor(255, 166, 201)},
		{"CarrotOrange", QColor(237, 145, 33)},
		{"Celadon", QColor(172, 225, 175)},
		{"Cerise", QColor( 222, 49, 99)},
		{"Cerulean", QColor(0, 123, 167)},
		{"CeruleanBlue", QColor(42, 82, 190)},
		{"Chartreuse", QColor(127, 255, 0)},
		{"ChartreuseYellow", QColor(223, 255, 0)},
		{"Chestnut", QColor(205, 92, 92)},
		{"Chocolate", QColor(123, 63, 0)},
		{"Cinnabar", QColor(227, 66, 52)},
		{"Cinnamon", QColor(210, 105, 30)},
		{"Cobalt", QColor(0, 71, 171)},
		{"Copper", QColor(184, 115, 51)},
		{"CopperRose", QColor(153, 102, 102)},
		{"Coral", QColor(255, 127, 80)},
		{"CoralRed", QColor(255, 64, 64)},
		{"Corn", QColor(251, 236, 93)},
		{"CornflowerBlue", QColor(100, 149, 237)},
		{"CosmicLatte", QColor(225, 248, 231)},
		{"Cream", QColor(255, 253, 208)},
		{"Crimson", QColor(220, 20, 60)},
		{"DarkBlue", QColor( 0, 0, 139)},
		{"DarkBrown", QColor(101, 67, 33)},
		{"DarkCerulean", QColor(8, 69, 126)},
		{"DarkChestnut", QColor(152, 105, 96)},
		{"DarkCoral", QColor(205, 91, 69)},
		{"DarkGoldenrod", QColor(184, 134, 11)},
		{"DarkGreen", QColor(1, 50, 32)},
		{"DarkKhaki", QColor(189, 183, 107)},
		{"DarkPastelGreen", QColor(3, 192, 60)},
		{"DarkPink", QColor(231, 84, 128)},
		{"DarkSalmon", QColor(233, 150, 122)},
		{"DarkSlateGray", QColor(47, 79, 79)},
		{"DarkSpringGreen", QColor(23, 114, 69)},
		{"DarkTan", QColor(145, 129, 81)},
		{"DarkTangerine", QColor(255, 168, 18)},
		{"DarkTurquoise", QColor(17, 96, 98)},
		{"DarkViolet", QColor(66, 49, 137)},
		{"DeepVerise", QColor(218, 50, 135)},
		{"DeepFuchsia", QColor(193, 84, 193)},
		{"DeepLilac", QColor(153, 85, 187)},
		{"DeepMagenta", QColor(204, 0, 204)},
		{"DeepPeach", QColor(255, 203, 164)},
		{"Denim", QColor(21, 96, 189)},
		{"DodgerBlue", QColor(30, 144, 255)},
		{"Ecru", QColor(194, 178, 128)},
		{"ElectricBlue", QColor(125, 249, 255)},
		{"ElectricIndigo", QColor(102, 0, 255)},
		{"Emerald", QColor(80, 200, 120)},
		{"Eggplant", QColor(153, 0, 102)},
		{"FaluRed", QColor(128, 24, 24)},
		{"FerrariRed", QColor(255, 28, 0)},
		{"FernGreen", QColor(79, 121, 66)},
		{"Flax", QColor(238, 220, 130)},
		{"ForestGreen", QColor(34, 139, 34)},
		{"FrenchRose", QColor(246, 74, 138)},
		{"Fuchsia", QColor(255, 0, 255)},
		{"FuchsiaPink", QColor(255, 119, 255)},
		{"Gamboge", QColor(228, 155, 15)},
		{"GoldMmetallic", QColor(212, 175, 55)},
		{"GoldWebGolden", QColor(255, 215, 0)},
		{"GoldenBrown", QColor(153, 101, 21)},
		{"GoldenYellow", QColor(255, 223, 0)},
		{"Goldenrod", QColor(218, 165, 32)},
		{"GreyAsparagus", QColor(70, 89, 69)},
		{"GreenYellow", QColor(173, 255, 47)},
		{"HanPurple", QColor(82, 24, 250)},
		{"Harlequin", QColor(63, 255, 0)},
		{"Heliotrope", QColor(223, 115, 255)},
		{"HollywoodCerise", QColor(244, 0, 161)},
		{"HotMagenta", QColor(255, 0, 204)},
		{"HotPink", QColor(255, 105, 180)},
		{"IndigoDdye", QColor(0, 65, 106)},
		{"IndigoWeb", QColor(75, 0, 130)},
		{"InternKleinBlue", QColor(0, 47, 167)},
		{"InternOrang", QColor(255, 79, 0)},
		{"IslamicGreen", QColor(0, 153, 0)},
		{"Ivory", QColor(255, 255, 240)},
		{"Jade", QColor(0, 168, 107)},
		{"KellyGreen", QColor(76, 187, 23)},
		{"Khaki", QColor(195, 176, 145)},
		{"KhakiX11", QColor(240, 230, 140)},
		{"Lavender", QColor(181, 126, 220)},
		{"LavenderBlue", QColor(204, 204, 255)},
		{"LavenderBlush", QColor(255, 240, 245)},
		{"LavenderGrey", QColor(189, 187, 215)},
		{"LavenderMagenta", QColor(238, 130, 238)},
		{"LavenderPink", QColor(251, 174, 210)},
		{"LavenderPurple", QColor(150, 120, 182)},
		{"LavenderRose", QColor(251, 160, 227)},
		{"Lemon", QColor(253, 233, 16)},
		{"LemonChiffon", QColor(255, 250, 205)},
		{"LightBlue", QColor(173, 216, 230)},
		{"Lilac", QColor(200, 162, 200)},
		{"Lime", QColor(191, 255, 0)},
		{"Linen", QColor(250, 240, 230)},
		{"Malachite", QColor(11, 218, 81)},
		{"Maroon", QColor(128, 0, 0)},
		{"MayaBlue", QColor(115, 194, 251)},
		{"Mauve", QColor(224, 176, 255)},
		{"MauveTaupe", QColor(145, 95, 109)},
		{"MediumBlue", QColor(0, 0, 205)},
		{"MediumCarmine", QColor(175, 64, 53)},
		{"MediumPurple", QColor(147, 112, 219)},
		{"MidnightBlue", QColor(0, 51, 102)},
		{"MintGreen", QColor(152, 255, 152)},
		{"MistyRose", QColor(255, 228, 225)},
		{"MossGreen", QColor(173, 223, 173)},
		{"MountbattenPink", QColor(153, 122, 141)},
		{"Mustard", QColor(255, 219, 88)},
		{"NavajoWhite", QColor(255, 222, 173)},
		{"NavyBlue", QColor(0, 0, 128)},
		{"Ochre", QColor(204, 119, 34)},
		{"OfficeGreen", QColor(0, 128, 0)},
		{"OldGold", QColor(207, 181, 59)},
		{"OldLace", QColor(253, 245, 230)},
		{"OldLavende", QColor(121, 104, 120)},
		{"OldRose", QColor(192, 46, 76)},
		{"Olive", QColor(128, 128, 0)},
		{"OliveDrab", QColor(107, 142, 35)},
		{"Olivine", QColor(154, 185, 115)},
		{"OrangeWeb", QColor(255, 165, 0)},
		{"OrangePeel", QColor(255, 160, 0)},
		{"OrangeRed", QColor(255, 69, 0)},
		{"Orchid", QColor(218, 112, 214)},
		{"PaleBlue", QColor(175, 238, 238)},
		{"PaleBrown", QColor(152, 118, 84)},
		{"PaleCarmine", QColor(175, 64, 53)},
		{"PaleChestnut", QColor(221, 173, 175)},
		{"PaleCornflowerBlue", QColor(171, 205, 239)},
		{"PaleMagenta", QColor(249, 132, 229)},
		{"PalePink", QColor(250, 218, 221)},
		{"PaleRredViole", QColor(219, 112, 147)},
		{"PapayaWhip", QColor(255, 239, 213)},
		{"PastelGreen", QColor(119, 221, 119)},
		{"PastelPink", QColor(255, 209, 220)},
		{"Peach", QColor(255, 229, 180)},
		{"PeachOrange", QColor(255, 204, 153)},
		{"PeachYellow", QColor(250, 223, 173)},
		{"Pear", QColor(209, 226, 49)},
		{"Periwinkle", QColor(204, 204, 255)},
		{"PersianBlue", QColor(28, 57, 187)},
		{"PersianGreen", QColor(0, 166, 147)},
		{"PersianIndigo", QColor(50, 18, 122)},
		{"PersianRed", QColor(204, 51, 51)},
		{"PersianPink", QColor(247, 127, 190)},
		{"PersianRose", QColor(254, 40, 162)},
		{"Persimmon", QColor(236, 88 ,0)},
		{"PineGreen", QColor(1, 121, 111)},
		{"PinkOrange", QColor(255, 153, 102)},
		{"PowderBlue", QColor(176, 224, 230)},
		{"Puce", QColor(204, 136, 153)},
		{"PrussianBlu", QColor(0, 49, 83)},
		{"PsychedelicPurple", QColor(221, 0 , 255)},
		{"Pumpkin", QColor(255, 117, 24)},
		{"Purple", QColor(128, 0, 128)},
		{"PurpleX11", QColor(160, 92, 240)},
		{"PurpleTaupe", QColor(80, 64, 77)},
		{"RawUmber", QColor(115, 74, 18)},
		{"RedViolet", QColor(199, 21, 133)},
		{"RobinEggBlue", QColor(0, 204, 204)},
		{"Rose", QColor(255, 0, 127)},
		{"RoseTaupe", QColor(144, 93, 93)},
		{"RoyalBlue", QColor(65, 105, 225)},
		{"RoyalPurple", QColor(107, 63, 160)},
		{"Russet", QColor(128, 70, 27)},
		{"Rust", QColor(183, 65, 14)},
		{"SafetyOrange", QColor(255, 102, 0)},
		{"Saffron", QColor(244, 196, 48)},
		{"Sapphire", QColor(8, 37, 103)},
		{"Salmon", QColor(255, 140, 105)},
		{"SandyBrown", QColor(244, 164, 96)},
		{"Sangria", QColor(146, 0, 10)},
		{"Scarlet", QColor(255, 36 ,0)},
		{"SchoolBusYellow", QColor(255, 216, 0)},
		{"SeaGreen", QColor(46, 139, 87)},
		{"Seashell", QColor(255, 245, 238)},
		{"SelectiveYellow", QColor(255, 186, 0)},
		{"Sepia", QColor(112, 66, 20)},
		{"ShamrockGreen", QColor(0, 158, 96)},
		{"ShockingPink", QColor(252, 15, 192)},
		{"Silver", QColor(192, 192, 192)},
		{"SkyBlue", QColor(135, 206, 235)},
		{"SlateGrey", QColor(112, 128, 144)},
		{"Smalt", QColor(0, 51, 153)},
		{"SpringGreen", QColor(0, 255, 127)},
		{"SteelBlue", QColor(70, 130, 180)},
		{"Tan", QColor(210, 180, 140)},
		{"Tangerine", QColor(242, 133, 0)},
		{"TangerineYellow", QColor(255, 204, 0)},
		{"Taupe", QColor(72, 60, 50)},
		{"TeaGreen", QColor(208, 240, 192)},
		{"TeaRose", QColor(248, 131, 194)},
		{"TeaRoseRed", QColor(244, 194, 194)},
		{"Teal", QColor(0, 128, 128)},
		{"TenneTawny", QColor(205, 87, 0)},
		{"TerraCotta", QColor(226, 114, 91)},
		{"Thistle", QColor(216, 191, 216)},
		{"Turquoise", QColor(48, 213, 200)},
		{"TyrianPurple", QColor(102, 2 , 60)},
		{"Ultramarine", QColor(18, 10, 143)},
		{"Vermilion", QColor(255, 77, 0)},
		{"Violet", QColor(139, 0, 255)},
		{"VioletWeb", QColor(238, 130, 238)},
		{"Viridian", QColor(64, 130, 109)},
		{"Wheat", QColor(245, 222, 179)},
		{"Wisteria", QColor(201, 160, 220)},
		{"Zinnwaldite", QColor(235, 194, 175)},
		{"White", QColor(255, 255, 255)}
	};

	static const int rgbTblSize = sizeof(rgbTbl) / sizeof(DrowColors);

	QColor QtDrawColors::getDrawColor(const char *name) {
		int i = 0;
		for (i = 0; i < rgbTblSize; i++) {
			if (name == rgbTbl[i].name)
				return rgbTbl[i].value;
		}
		return rgbTbl[0].value;
	}

	QColor QtDrawColors::getDrawColor(int i) {
		return rgbTbl[i % rgbTblSize].value;
	}

	QStringList QtDrawColors::getColorNames() {
		int i = 0;
		QStringList lst;
		for (i = 0; i < rgbTblSize; i++)
			lst << QLatin1String(rgbTbl[i].name);
		return lst;
	}

}

