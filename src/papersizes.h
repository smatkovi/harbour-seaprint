#ifndef PAPERSIZES_H
#define PAPERSIZES_H
#include <QMap>
#include <QSizeF>

// Qt 4.7's QMap has no initializer-list constructor, so the table is filled in
// rather than declared. The build that uses it is the same either way.
static QMap<QString, QString> makeCalligraPaperSizes()
{
    QMap<QString, QString> sizes;
    sizes.insert("iso_a0_841x1189mm", "A0");
    sizes.insert("iso_a1_594x841mm", "A1");
    sizes.insert("iso_a2_420x594mm", "A2");
    sizes.insert("iso_a3_297x420mm", "A3");
    sizes.insert("iso_a4_210x297mm", "A4");
    sizes.insert("iso_a5_148x210mm", "A5");
    sizes.insert("iso_a6_105x148mm", "A6");
    sizes.insert("iso_a7_74x105mm", "A7");
    sizes.insert("iso_a8_52x74mm", "A8");
    sizes.insert("iso_a9_37x52mm", "A9");
    sizes.insert("iso_b0_1000x1414mm", "B0");
    sizes.insert("iso_b1_707x1000mm", "B1");
    sizes.insert("iso_b2_500x707mm", "B2");
    sizes.insert("iso_b3_353x500mm", "B3");
    sizes.insert("iso_b4_250x353mm", "B4");
    sizes.insert("iso_b5_176x250mm", "B5");
    sizes.insert("iso_b6_125x176mm", "B6");
    sizes.insert("iso_b7_88x125mm", "B7");
    sizes.insert("iso_b8_62x88mm", "B8");
    sizes.insert("iso_b9_44x62mm", "B9");
    sizes.insert("iso_b10_31x44mm", "B10");
    sizes.insert("iso_c5_162x229mm", "C5E");
    sizes.insert("na_number-10_4.125x9.5in", "Comm10E");
    sizes.insert("iso_dl_110x220mm", "DLE");
    sizes.insert("na_executive_7.25x10.5in", "Executive");
    sizes.insert("om_folio_210x330mm", "Folio");
    sizes.insert("na_ledger_11x17in", "Ledger");
    sizes.insert("na_legal_8.5x14in", "Legal");
    sizes.insert("na_letter_8.5x11in", "Letter");
    sizes.insert("na_ledger_11x17in", "Tabloid");
    return sizes;
}

static QMap<QString, QString> CalligraPaperSizes = makeCalligraPaperSizes();



#endif // PAPERSIZES_H
