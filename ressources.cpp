#include "ressources.h"

Ressources* Ressources::instance = Q_NULLPTR;

Ressources* Ressources::I()
{
    if( !instance )
        instance = new Ressources();
    return instance;
}

Ressources::Ressources() {
m_head =
"<head>"
    "<meta name=\"qrichtext\" content=\"1\">"
    "<style type=\"text/css\">"
        "p {margin-top:0px; margin-bottom:0px;margin-left: 0px; font-size:9pt},li { white-space: pre-wrap; }"
    "</style>"
    "<style>"
        ".tableR {"
                "width: 55%;"
                "float:left;"
                 "}"

        ".tableR table {"
                            "border: none #ffffff;"
                            "height: 100%;"
                            "width: 55%;"
                            "table-layout: fixed;"
                            "border-collapse: collapse;"
                            "border-spacing: px;"
                            "text-align: left;"
                       "}"
        ".tableR th, .tableR td {"
                            "border: none #ffffff;"
                            "background-color: #ffffff;"
                            "color: #000000;"
                            "padding: 0px;"
                            "width: 50%;"
                            "}"
        ".tableR .spancol {"
                       "column-span: all;"
                       "}"
        ".tableR .fontAdeli {"
                            "font-size:8pt;"
                            "font-style:normal;"
                            "}"
    "</style>"
    "<style>"
        ".tableL {"
                "width: 43%;"
                "float:right;"
                "}"
        ".tableL table {"
                            "border: none #ffffff;"
                            "height: 100%;"
                            "width: 43%;"
                            "table-layout: fixed;"
                            "border-collapse: collapse;"
                            "border-spacing: px;"
                            "text-align: right;"
                            "}"
        ".tableL th, .tableL td {"
                            "border: none #ffffff;"
                            "background-color: #ffffff;"
                            "color: #000000;"
                            "padding: 0px;"
                            "width: 24%;"
                       "}"
        ".tableL .alignR {"
                            "text-align: right;"
                            "border-style: none;"
                            "}"
    "</style>"
    "<style>"
        ".column {"
                    "display:inline-block;"
                    "width:50%;"
                    "float:left;"
                "}"
    "</style>"
"</head>";
}

QString Ressources::HeaderOrdo()
{
    QString  HeadOrdo =
"<html>" +
    m_head +
"<body style=\"font-family:{{POLICE}}; font-size:9pt; font-style:italic;\">"
"<div class=\"column\">"
    "<table class=\"tableR\">"
        "<tbody>"
            "<tr>"
                "<td style=\"font-size:10pt; font-weight:600; colspan:2;\">"
                    "{{TITREUSER}}"
                "</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\";>{{SPECIALITE}}</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\";>{{ADRESSE}}</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\";>{{CPVILLE}}</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\";>{{TEL}}</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:5pt\";>{{NUMSS}}</td>"
            "</tr>"
        "</tbody>"
    "</table>"
"</div>"
"<div class=\"column\">"
    "<table class=\"tableL\">"
        "<tbody>"
            "<tr>"
                "<td class = \"alignR\">"
                    "<!--date-->{{DATE}}<!--date-->"
                "</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{TITRE1}}</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{TITRE}}</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{PRENOM PATIENT}} {{NOM PATIENT}}</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{DDN}}</td>"
            "</tr>"
        "</tbody>"
    "</table>"
"</div>"
"</body>"
"</html>";
    return HeadOrdo;

}

QString Ressources::HeaderOrdoWithBarCode()
{
    QString  HeadOrdo =
"<html>" +
   m_head +
"<body style=\"font-family:{{POLICE}}; font-size:9pt; font-style:italic;\">"
"<div class=\"column\">"
    "<table class=\"tableR\">"
        "<tbody>"
            "<tr>"
                "<td style=\"font-size:10pt; font-weight:600; colspan:2;\">"
                    "{{TITREUSER}}"
                "</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\">{{SPECIALITE}}</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\">{{ADRESSE}}</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\">{{CPVILLE}}</td>"
            "</tr>"
            "<tr>"
                "<td style=\"font-size:8pt\">{{TEL}}</td>"
            "</tr>"
        "</tbody>"
    "</table>"
"</div>"
"<div class=\"column\">"
    "<table class=\"tableL\">"
        "<tbody>"
            "<tr>"
                "<td class = \"alignR\";>""<!--date-->{{DATE}}<!--date-->""</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{TITRE1}}</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{TITRE}}</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{PRENOM PATIENT}} {{NOM PATIENT}}</td>"
            "</tr>"
            "<tr>"
                "<td class = \"alignR\">{{DDN}}</td>"
            "</tr>"
        "</tbody>"
    "</table>"
"</div>"
"</body>"
"</html>";
    return HeadOrdo;
}

QString Ressources::HeaderOrdoALD()
{
QString  HeadOrdoALD =
"<html>"
"<head>"
    "<meta name=\"qrichtext\" content=\"1\" />"
    "<meta charset=\"utf-8\" />"
    "<style type=\"text/css\">"
    "</style>"
"</head>"
"<body style=\"font-family:{{POLICE}}; font-size:9pt; font-weight:400; font-style:italic;\">"
"<table cellpadding=\"0px\" cellspacing=\"10px\" style= \"text-align:center;\">"
    "<CAPTION ALIGN=top>"
        "<span style=\"font-size:16pt;color:blue; font-style:normal;\">Ordonnance Bizone</span><br />"
        "<span style=\"font-size:5pt; color:blue;\">Articles L. 322-3, 3° et 4°, L. 324-1 et R. 161-45 du Code de la sécurité sociale.</span>"
    "</CAPTION>"
    "<tr>"
        "<td align=\"left\" width=\"" HTML_LARGEUR_ENTETE_DROITE_ALD "\" style=\"font-size:8pt; font-weight:400;\">"
            "<br />"
            "<span style=\"font-size:9pt; font-weight:600;\">"
                "{{TITREUSER}}<br />"
            "</span>"
            "<span style=\"color:black;\">"
                "{{SPECIALITE}}""<br />"
                "{{ADRESSE}}<br />"
                "{{CPVILLE}}<br />"
                "{{TEL}}"
            "</span>"
        "</td>"
        "<td valign=\"middle\" align=\"right\" width=\"" HTML_LARGEUR_ENTETE_DROITE_ALD "\" style=\"font-size:7pt; color:blue;\">"
            "l'étiquette du patient est à coller ici"
        "</td>"
    "</tr>"
"</body>"
"</html>";
    return HeadOrdoALD;
}

QString Ressources::BodyOrdo()
{
    QString BodyOrdo =
"<html>"
"<body>"
    "<br /><br /><br /><br /><br /><br />"
    "<p>"
        "<div align=\"justify\" >"
            "<span style=\"font-size:9pt\">"
                "{{TEXTE ORDO}}"
            "</span>"
        "</div>"
    "</p>"
"</body>"
"</html>";
    return BodyOrdo;
}

QString Ressources::BodyOrdoALD()
{
    QString BodyOrdoALD =
"<html>" +
    m_head +
"<body style==\"font-family:{{POLICE}}; font-size:9pt; font-weight:400; font-style:italic;\";>"
"<div class=\"column\";>"
    "<table class=\"tableR\";>"
        "<tbody>"
            "<tr>"
                "<td style=\"font-size:9pt\">"
                    "{{PRENOM PATIENT}} {{NOM PATIENT}}<br />"
                "</td>"
            "</tr>"
        "</tbody>"
    "</table>"
"</div>"
"<div class=\"column\";>"
    "<table class=\"tableL\";>"
        "<tbody>"
            "<tr>"
                "<td class = \"alignR\" style=\"font-size:9pt\">"
                    "<!--date-->{{DATE}}<!--date-->"
                "</td>"
            "</tr>"
        "</tbody>"
    "</table>"
"</div>"
    "<table width = \"100%\" border = \"1\" style=\"clear:both\">"
        "<tr>"
            "<td>"
                "<div align = \"center\">"
                    "<p>"
                        "<span style=\"font-size:8pt; font-style:normal;text-align:center; color:blue;\";>"
                            "Prescriptions relatives au traitement de l'affection de longue durée reconnue (liste ou hors liste)<br />"
                            "(AFFECTION EXONERANTE)"
                        "</span>"
                    "</p>"
                "</div>"
            "</td>"
        "</tr>"
    "</table>"
    "<table style=\"margin-left:10px\" cellpadding=\"5\">"
        "<tr>"
            "<td>"
                "{{TEXTE ORDO}}<br /><br /><br /><br /><br /><br /><br />"
            "</td>"
        "</tr>"
    "</table>"
    "<table style = \"position:absolute;top:600px;\" width = \"100%\" border = \"1\">"
        "<tr>"
            "<td>"
                "<div align = \"center\">"
                    "<p>"
                        "<span style=\"font-size:8pt; font-style:normal;text-align:center; color:blue;\">"
                            "Prescriptions SANS RAPPORT avec l'affection de longue durée reconnue<br />"
                            "(MALADIES INTERCURRENTES)"
                        "</span>"
                    "</p>"
                "</div>"
            "</td>"
        "</tr>"
    "</table>"
    "<p>"
        "{{TEXTE ORDO HORS ALD}}"
    "</p>"
"</body>"
"</html>";
    return BodyOrdoALD;
}

QString Ressources::FooterOrdo()
{
    QString FooterOrdo =
    "<html>"
        "{{DUPLI}}"
        "<p align=\"center\"; style = \"margin-top:0px; margin-bottom:0px;\">"
            "<span style=\"font-size:6pt\">"
                "{{AGA}}"
            "</span>"
        "</p>"
    "</html>";
    return FooterOrdo;
}


QString Ressources::FooterOrdoLunettes()
{
    QString FooterOrdoLun =
"<html>"
    "{{DUPLI}}"
    "<p align=\"justify\"; style = \"margin-top:0px; margin-bottom:0px;\">"
        "<span style=\"font-size:8pt\">"
            "<i>La puissance de vos verres correcteurs a &eacute;t&eacute; &eacute;tablie par votre {{METIER}} qui en prend la responsabilit&eacute;. Il est vivement conseill&eacute; que la r&eacute;alisation de vos lunettes soit faite par un opticien dipl&ocirc;m&eacute;. Cette r&eacute;alisation comporte le conseil pour le choix de la monture, les prises de mesures pr&eacute;cises n&eacute;cessaires au montage (la mesure de l'&eacute;cart pupillaire ne suffit pas), le montage des verres, et l'adaptation des lunettes lors de leur livraison.<br /></i>"
        "</span>"
    "</p>"
    "<p align=\"center\"; style = \"margin-top:0px; margin-bottom:0px;\">"
        "<span style=\"font-size:6pt\">"
            "{{AGA}}"
        "</span>"
    "</p>"
"</html>";
    return FooterOrdoLun;
}
