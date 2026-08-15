#!/usr/bin/env python3
"""Injecte les images d'implants d'IOLCon dans IOLexport2.3.xml.

Usage : ./maj_images_iolcon.py page_iolcon.html IOLexport2.3.xml [-b url] [-c cache]

Lit la page du tableau IOLCon enregistrée depuis le navigateur, télécharge l'image
de chaque implant, la ramène à 240 px de côté et la comprime en JPEG sous 8 Ko, puis
l'écrit en base64 dans une balise <Image> de son <Lens>. Le cache évite de retélécharger
d'une exécution à l'autre ; une balise <Image> déjà présente est remplacée, jamais doublée.
Voir README.md pour la marche à suivre complète.
"""
import argparse, base64, io, re, sys, time
from pathlib import Path
from urllib.parse import quote
from urllib.request import urlopen
from PIL import Image

COTE_MAX = 240                                          # la plus grande fenêtre de Rufus fait 160x180 px
POIDS_MAX = 8192


def images_de_la_page(html):
    """Renvoie {identifiant IOLCon: chemin de l'image} lu dans les lignes du tableau."""
    trouvees = {}
    for m in re.finditer(r'<tr id="row(\d+)"(.*?)</tr>', html, re.S):
        img = re.search(r'src="(userdata/images/[^"]+)"', m.group(2))
        if img:
            trouvees[m.group(1)] = img.group(1)
    return trouvees


def comprime(octets):
    """Renvoie les octets JPEG de l'image, réduite et comprimée sous le seuil."""
    img = Image.open(io.BytesIO(octets))
    if img.mode in ("RGBA", "LA", "P"):                 # aplatir la transparence sur blanc,
        img = img.convert("RGBA")                       # sinon le JPEG la rend noire
        fond = Image.new("RGB", img.size, (255, 255, 255))
        fond.paste(img, mask=img.split()[-1])
        img = fond
    else:
        img = img.convert("RGB")
    if max(img.size) > COTE_MAX:
        r = COTE_MAX / max(img.size)
        img = img.resize((max(1, round(img.width * r)), max(1, round(img.height * r))), Image.LANCZOS)
    for q in range(85, 19, -5):
        buf = io.BytesIO()
        img.save(buf, "JPEG", quality=q, optimize=True)
        if buf.tell() <= POIDS_MAX:
            break
    return buf.getvalue()


def recupere(idlens, chemin, base, cache, echecs):
    """Renvoie l'image comprimée, prise au cache ou téléchargée puis mise en cache."""
    fichier = cache / f"{idlens}.jpg"
    if fichier.is_file() and fichier.stat().st_size:
        return fichier.read_bytes()
    url = base.rstrip("/") + "/" + quote(chemin, safe="/")
    try:
        with urlopen(url, timeout=30) as r:
            octets = comprime(r.read())
        time.sleep(0.3)                                 # on ne bourrine pas le site
    except Exception as e:
        echecs.append(f"{idlens} {chemin} ({e})")
        return None
    fichier.write_bytes(octets)
    return octets


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("page", type=Path, help="page du tableau IOLCon enregistrée")
    ap.add_argument("xml", type=Path, help="IOLexport2.3.xml, modifié sur place")
    ap.add_argument("-b", "--base", default="https://iolcon.org", help="racine du site")
    ap.add_argument("-c", "--cache", type=Path, default=Path("cache_images_iolcon"))
    args = ap.parse_args()

    args.cache.mkdir(parents=True, exist_ok=True)
    liens = images_de_la_page(args.page.read_text(encoding="utf8", errors="replace"))
    if not liens:
        sys.exit("aucune image trouvée dans la page : mauvais fichier ?")
    print(f"{len(liens)} images référencées dans la page")

    xml = args.xml.read_text(encoding="utf8")
    echecs, compte = [], {"injectees": 0, "sansimage": 0, "poids": 0}

    def traite(m):
        idlens, attributs, corps = m.group(1), m.group(2), m.group(3)
        corps = re.sub(r"\s*<Image>.*?</Image>", "", corps, flags=re.S)
        chemin = liens.get(idlens)
        octets = recupere(idlens, chemin, args.base, args.cache, echecs) if chemin else None
        if octets:
            compte["injectees"] += 1
            compte["poids"] += len(octets)
            b64 = base64.b64encode(octets).decode()
            return f'<Lens id="{idlens}"{attributs}>{corps}\t\t<Image>{b64}</Image>\n\t</Lens>'
        compte["sansimage"] += 1
        return f'<Lens id="{idlens}"{attributs}>{corps}</Lens>'

    args.xml.write_text(re.sub(r'<Lens id="(\d+)"([^>]*?)>(.*?)</Lens>', traite, xml, flags=re.S),
                        encoding="utf8")

    print(f"implants         : {compte['injectees'] + compte['sansimage']}")
    print(f"images injectées : {compte['injectees']}")
    print(f"sans image       : {compte['sansimage']}")
    print(f"poids des images : {compte['poids']/1048576:.1f} Mo")
    print(f"XML produit      : {args.xml} — {args.xml.stat().st_size/1048576:.1f} Mo")
    for e in echecs:
        print("echec:", e)


if __name__ == "__main__":
    main()
