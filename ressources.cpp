#include "ressources.h"

Ressources::Ressources() {}

Ressources* Ressources::instance = Q_NULLPTR;

Ressources* Ressources::I()
{
    if( !instance )
        instance = new Ressources();
    return instance;
}

QString Ressources::HeaderOrdo()
{
QString  HeadOrdo =
"<html>"
    "<head>"
    "<meta name=\"qrichtext\" content=\"1\">"
    "<style type=\"text/css\">"
        "p {margin-top:0px; margin-bottom:0px;margin-left: 0px; font-size:9pt},li { white-space: pre-wrap; }"
    "</style>"
"</head>"
"<body style=\"font-family:{{POLICE}}; font-size:9pt; font-style:italic;\">"
    "<table style=\"float:left;\">"
        "<tr>"
            "<td width=\"{{LARGEURG}}\">"
                "<p>"
                    "<span style=\"font-size:10pt; font-weight:600;\">"
                        "{{TITREUSER}}<br />"
                    "</span>"
                    "<span style=\"font-size:8pt;\">"
                        "{{SPECIALITE}}<br />"
                        "{{ADRESSE}}<br />"
                        "{{CPVILLE}}<br />{{TEL}}<br />"
                        "</span>"
                    "<span style=\"font-size:5pt;\">"
                        "{{NUMSS}}"
                    "</span>"
                "</p>"
            "</td>"
        "</tr>"
    "</table>"
    "<table style=\"float:right;\">"
        "<tr>"
            "<td width=\"{{LARGEURD}}\">"
                "<p align=\"right\">"
                    "<span style=\"font-size:10pt;\">"
                        "<!--date-->{{DATE}}<!--date--><br /><br />"
                        "{{TITRE1}}<br />"
                        "{{TITRE}}<br />"
                        "{{PRENOM PATIENT}} {{NOM PATIENT}}<br />"
                        "{{DDN}}"
                    "</span>"
                "</p>"
            "</td>"
        "</tr>"
    "</table>"
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
                "{{TEL}}<br />"
            "</span>"
            "<span style=\"font-size:5pt; color:black;\">"
                "{{NUMSS}}"
            "</span>"
        "</td>"
        "<td valign=\"middle\" align=\"right\" width=\"" HTML_LARGEUR_ENTETE_DROITE_ALD "\" style=\"font-size:7pt; color:blue;\">"
            "l'étiquette du patient est à coller ici"
        "</td>"
    "</tr>"
    "<tr>"
        "<td align=\"left\">"
            "<br />"
            "{{PRENOM PATIENT}} {{NOM PATIENT}}"
        "</td>"
        "<td align=\"right\">"
            "<br />"
            "<!--date-->"
                "{{DATE}}"
            "<!--date-->"
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
"<html>"
"<head>"
    "<meta name=\"qrichtext\" content=\"1\" />"
    "<meta charset=\"utf-8\" />"
"</head>"
"<body style=\" font-family:'Comic Sans MS'; font-size:11pt; font-style:italic;\">"
    "<table width = \"100%\" border = \"1\" style=\"clear:both;\">"
        "<tr>"
            "<td>"
                "<div align = \"center\">"
                    "<p>"
                        "<span style=\"font-size:8pt; font-style:normal;text-align:center; color:blue;\">"
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
            "<i>La puissance de vos verres correcteurs a &eacute;t&eacute; &eacute;tablie par votre ophtalmologiste qui en prend la responsabilit&eacute;. Il est vivement conseill&eacute; que la r&eacute;alisation de vos lunettes soit faite par un opticien dipl&ocirc;m&eacute;. Cette r&eacute;alisation comporte le conseil pour le choix de la monture, les prises de mesures pr&eacute;cises n&eacute;cessaires au montage (la mesure de l'&eacute;cart pupillaire ne suffit pas), le montage des verres, et l'adaptation des lunettes lors de leur livraison.<br /></i>"
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
