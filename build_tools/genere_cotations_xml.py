#!/usr/bin/env python3
"""Régénère Cotations.xml à partir du fichier CCAM officiel (.xls de la CNAM).

Usage : python3 genere_cotations_xml.py CCAM_V84_VF.xls Cotations.xml [sortie.xml]

La partie CCAM (version + actes origin=ccam) est reconstruite depuis le .xls ;
la partie NGAP (version, valeurs AMY, actes origin=ngap) est recopiée telle
quelle depuis le Cotations.xml existant, elle ne vient pas du .xls.
"""

import re
import sys
import xlrd

RGX_CODE = re.compile(r'^[A-Z]{4}[0-9]{3}$')            # un acte CCAM : 4 lettres + 3 chiffres


def echappe(txt):
    return txt.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')


def montant(val):
    """Le tarif du .xls : nombre, vide ou « Non pris en charge » -> 0.00 (« 248, 15 » : coquille CNAM)."""
    try:
        return '%.2f' % float(re.sub(r'\s', '', str(val)).replace(',', '.'))
    except ValueError:
        return '0.00'


def lit_xls(chemin):
    """Retourne (version, [(code, nom, optam, nonoptam)]) dans l'ordre du fichier.

    Les colonnes sont repérées par leur en-tête : elles ont bougé entre V83 et V84.
    """
    feuille = xlrd.open_workbook(chemin).sheet_by_index(0)

    ligne_entete, icode, itexte, itarifs = None, None, None, []
    for r in range(min(10, feuille.nrows)):
        valeurs = [str(c.value).strip() for c in feuille.row(r)]
        if 'Code' in valeurs:
            ligne_entete = r
            icode = valeurs.index('Code')
            itexte = valeurs.index('Texte')
            itarifs = [i for i, v in enumerate(valeurs) if v.startswith('Tarif')]
            break
    if ligne_entete is None or len(itarifs) < 2:
        raise SystemExit("en-tête (Code / Texte / 2 colonnes Tarif) introuvable dans " + chemin)

    version = ''
    for r in range(ligne_entete):
        for c in feuille.row(r):
            trouve = re.search(r'version\s+(\d+)', str(c.value), re.IGNORECASE)
            if trouve:
                version = trouve.group(1)
    if version == '':
        raise SystemExit("version CCAM introuvable dans l'en-tête de " + chemin)

    actes = []
    for r in range(ligne_entete + 1, feuille.nrows):
        code = str(feuille.cell(r, icode).value).strip()
        if not RGX_CODE.match(code):
            continue
        actes.append((code,
                      str(feuille.cell(r, itexte).value).strip(),
                      montant(feuille.cell(r, itarifs[0]).value),
                      montant(feuille.cell(r, itarifs[1]).value)))
    return version, actes


def bloc_ngap(chemin_xml):
    """Extrait du Cotations.xml existant le bloc <NGAP> et les actes origin=ngap, à recopier tels quels."""
    xml = open(chemin_xml, encoding='utf-8').read()
    ngap = re.search(r'\t<NGAP>.*?\t</NGAP>\n', xml, re.S)
    if not ngap:
        raise SystemExit('bloc <NGAP> introuvable dans ' + chemin_xml)
    actes = [a for a in re.findall(r'\t<Acte>.*?\t</Acte>\n', xml, re.S) if '<origin>ngap</origin>' in a]
    return ngap.group(0), ''.join(actes)


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    source_xls, source_xml = sys.argv[1], sys.argv[2]
    sortie = sys.argv[3] if len(sys.argv) > 3 else source_xml

    version, actes = lit_xls(source_xls)
    ngap, actes_ngap = bloc_ngap(source_xml)

    lignes = ['<?xml version="1.0" encoding="UTF-8" ?>\n<Cotations>\n',
              '\t<CCAM>\n\t\t<Version>%s</Version>\n\t</CCAM>\n' % version,
              ngap]
    for code, nom, optam, nonoptam in actes:
        lignes.append('\t<Acte>\n\t\t<code>%s</code>\n\t\t<nom>%s</nom>\n'
                      '\t\t<optam>%s</optam>\n\t\t<nonoptam>%s</nonoptam>\n'
                      '\t\t<origin>ccam</origin>\n\t</Acte>\n'
                      % (code, echappe(nom), optam, nonoptam))
    lignes.append(actes_ngap)
    lignes.append('</Cotations>\n')

    open(sortie, 'w', encoding='utf-8').write(''.join(lignes))
    print('%s : CCAM version %s, %d actes CCAM + %d actes NGAP'
          % (sortie, version, len(actes), actes_ngap.count('<Acte>')))


if __name__ == '__main__':
    main()
