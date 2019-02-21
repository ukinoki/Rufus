/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ICONS_H
#define ICONS_H

#include <QMap>
#include <QIcon>
#include <QPixmap>

/**
* \file Icons.h
* \brief Cette classe gére l'ensemble des icônes
* \author Alexanre.D
* \version 0.1
* \date 7 juin 2018
*
* Cette classe est utilisée de manière uniquement static.
* Cette classe référence l'ensemble des icônes utilisées, ainsi
* que les fonctions de gestions des icônes.
*
*/

class Icons
{
private:
    static QMap<QString,QPixmap> m_mapPixmap;
    static QPixmap foundPX(QString name, QString url)
    {
        if( !m_mapPixmap.contains(name) )
            m_mapPixmap[name] = QPixmap(url);
        return m_mapPixmap[name];
    }

    static QMap<QString,QIcon> m_mapIcon;
    static QIcon foundIC(QString name, QString url, QSize size = QSize())
    {
        if( size.isValid() )
            name += "_"+QString::number(size.width())+"x"+QString::number(size.height());

        if( !m_mapIcon.contains(name) )
        {
            QIcon ic;
            ic.addFile(url, size, QIcon::Normal, QIcon::Off);
            m_mapIcon[name] = ic;
        }
        return m_mapIcon[name];
    }
    static QIcon foundIC(QString name, QPixmap px, QSize size = QSize())
    {
        if( size.isValid() )
            px = px.scaled(size.width(), size.height());

        name += "_"+QString::number(px.width())+"x"+QString::number(px.height());
        if( !m_mapIcon.contains(name) )
        {
             QIcon ic;
             ic.addPixmap(px, QIcon::Normal, QIcon::Off);
             m_mapIcon[name] = ic;
        }
        return m_mapIcon[name];
    }

public:
    /**
     * QPixmap
     */
    static QPixmap pxAdministratif() { return foundPX("pxAdministratif", "://Administration.png"); }
    static QPixmap pxAnnuler() { return foundPX("pxAnnuler", "://cancel.png"); }
    static QPixmap pxAnnulerAppuye() { return foundPX("pxAnnulerAppuye", "://cancelpushed.png"); }
    static QPixmap pxApres() { return foundPX("pxApres", "://button_blue_play.png"); }
    static QPixmap pxAutoref() { return foundPX("pxAutoref", "://autoref.png"); }
    static QPixmap pxAVTest() { return foundPX("pxAVTest", "://AVTest1.png"); }

    static QPixmap pxBlackCheck() { return foundPX("pxBlackCheck", "://blackcheck.png"); }

    static QPixmap pxCheck() { return foundPX("pxCheck", "://check.png"); }
    static QPixmap pxConversation() { return foundPX("pxMessage", "://answer.png"); }
    static QPixmap pxCreditCard() { return foundPX("pxCard", "://credit_cards.png"); }

    static QPixmap pxDetente() { return foundPX("pxDetente", "://beach.png"); }
    static QPixmap pxDeverouiller() { return foundPX("pxDeverouiller", "://Unlock.png"); }
    static QPixmap pxDown() { return foundPX("pxDown", "://down.png"); }

    static QPixmap pxEditer() { return foundPX("pxEditer", "://edit.png"); }
    static QPixmap pxEnregistrer() { return foundPX("pxEnregistrer", "://Record.png"); }
    static QPixmap pxError() { return foundPX("pxError", "://error.png"); }

    static QPixmap pxImprimer() { return foundPX("pxImprimer", "://Imprimer.png"); }
    static QPixmap pxInformation() { return foundPX("pxInformation", "://information.png"); }

    static QPixmap pxKiller() { return foundPX("pxKiller", "://killer.png"); }

    static QPixmap pxListe() { return foundPX("pxListe", "://list_all_participants.png"); }
    static QPixmap pxLensMeter() { return foundPX("pxLensMeter", "://lensmeter.png"); }
    static QPixmap pxLoupe() { return foundPX("pxLoupe", "://search.png"); }

    static QPixmap pxOups() { return foundPX("pxOups", "://damn-icon.png"); }

    static QPixmap pxPoubelle() { return foundPX("pxPoubelle", "://trash.png"); }

    static QPixmap pxQuestion() { return foundPX("pxQuestion", "://question.png"); }
    static QPixmap pxQwant() { return foundPX("pxQwant", "://qwant.png"); }

    static QPixmap pxReception() { return foundPX("pxReception", "://reception_icon.png"); }
    static QPixmap pxRefracteur() { return foundPX("pxRefracteur", "://phoropter.png"); }

    static QPixmap pxSalleAttente() { return foundPX("pxSalleAttente", "://waiting_room.png"); }
    static QPixmap pxSplash() { return foundPX("pxSplash", "://rufus3.jpg"); }
    static QPixmap pxSuicide() { return foundPX("pxSuicide", "://suicide.png"); }
    static QPixmap pxSwitchLeft() { return foundPX("pxSwitchLeft", "://switch-left.png"); }
    static QPixmap pxSwitchRight() { return foundPX("pxSwitchRight", "://switch-right.png"); }

    static QPixmap pxTonometre() { return foundPX("pxTonometre", "://airtonometer.png"); }
    static QPixmap pxVerrouiller() { return foundPX("pxVerrou", "://Lock.png"); }

    static QPixmap pxWorkInProgress() { return foundPX("pxWorkInPorgress", "://work-in-progress.png").scaledToWidth(150); } //TODO : ??? pourquoi scaledToWidth (Alexandre) // je ne me rappelle plus (Serge)

    static QPixmap pxZoomIn() { return foundPX("pxZoomIn", "://Zoom-In.png"); }
    static QPixmap pxZoomOut() { return foundPX("pxZoomOut", "://Zoom-Out.png"); }






    /**
     * QIcon
     */
    static QIcon icSilhouette() { return foundIC("icSilhouette", "://silhouette.png", QSize(20, 20)); }
    static QIcon icMan() { return foundIC("icMan", "://man.png"); }
    static QIcon icWoman() { return foundIC("icWoman", "://women.png", QSize(30,30)); }
    static QIcon icGirl() { return foundIC("icGirl", "://girl.png", QSize(25, 25)); }
    static QIcon icBoy() { return foundIC("icBoy", "://boy.png", QSize(25, 25)); }
    static QIcon icKid() { return foundIC("icKid", "://kid.png"); }
    static QIcon icBaby() { return foundIC("icBaby", "://baby.png", QSize(25, 25)); }
    static QIcon getIconAge(QString img)
    {
        if (img == "man")      return icMan();
        if (img == "women")    return icWoman();
        if (img == "girl")     return icGirl();
        if (img == "boy")      return icBoy();
        if (img == "kid")      return icKid();
        if (img == "baby")     return icBaby();

        return icSilhouette();
    }

    static QIcon icCopy() { return foundIC("icCopy", "://copy.png"); }
    static QIcon icCut() { return foundIC("icCut", "://cut.ico"); }
    static QIcon icPaste() { return foundIC("icPaste", "://paste.ico"); }

    static QIcon icAjouter() { return foundIC("icInsert", "://ComPlus.png", QSize(45,45)); }
    static QIcon icEditer(QSize size = QSize(25,25)) { return foundIC("icModify", pxEditer(), size); }
    static QIcon icRetirer() { return foundIC("icSuppr", "://ComMoins.png", QSize(45,45)); }

    static QIcon icAdministratif(QSize size = QSize()) { return foundIC("icAdministratif", pxAdministratif(), size); }
    static QIcon icAlcool() { return foundIC("icAlcool", "://beer.png", QSize(30,30)); }
    static QIcon icAnnuler(QSize size = QSize()) { return foundIC("icAnnuler", pxAnnuler(), size); } //30
    static QIcon icAnnulerAppuye(QSize size = QSize()) { return foundIC("icAnnuler", pxAnnulerAppuye(), size); } //30
    static QIcon icApres() { return foundIC("icApres", pxApres()); }
    static QIcon icAvant() { return foundIC("icAvant", "://button_blue_rear.png"); }
    static QIcon icArchive() { return foundIC("icArchive", "://Archives.png"); }
    static QIcon icAttente() { return foundIC("icAttente", "://attente2.png"); }

    static QIcon icBack(QSize size = QSize(25,25)) { return foundIC("icBack", "://back.png", size); }
    static QIcon icBackup() { return foundIC("icBackup", "://Backup.png", QSize(45,45)); }
    static QIcon icBlackCheck() { return foundIC("icBlackCheck", pxBlackCheck(), QSize(45,45)); }
    static QIcon icBlockCenter() { return foundIC("icBlockCenter", "://align-center.png", QSize(45,45)); }
    static QIcon icBlockJustify() { return foundIC("icBlockJustify", "://align-justify.png", QSize(45,45)); }
    static QIcon icBlockLeft() { return foundIC("icBlockLeft", "://align-left.png", QSize(45,45)); }
    static QIcon icBlockRight() { return foundIC("icBlockRight", "://align-right.png", QSize(45,45)); }

    static QIcon icCheck() { return foundIC("icCheck", "://check.png", QSize(45,45)); }
    static QIcon icCheckOblig() { return foundIC("icCheckOblig", "://Check-icon.png", QSize(45,45)); }
    static QIcon icCinema() { return foundIC("icCinema", "://movie.png"); }
    static QIcon icClock() { return foundIC("icClock", "://Clock.png", QSize(45,45)); }
    static QIcon icComputer() { return foundIC("icComputer", "://Computer.png", QSize(45,45)); }
    static QIcon icContact() { return foundIC("icContact", "://contact.png"); }
    static QIcon icCPS() { return foundIC("icCPS", ""); } //FIXME : !!! manque icon CPS
    static QIcon icCreditCard() { return foundIC("icCredit",  pxCreditCard(), QSize(25,25));}
    static QIcon icCreer() { return foundIC("icCreer", "://toolbar_new_folder.png"); }

    static QIcon icDate() { return foundIC("icDate", "://calendar2.png"); }
    static QIcon icDoctor() { return foundIC("icDoctor", "://Doctor-icon.png"); }
    static QIcon icDoNotRecord(QSize size = QSize()) { return foundIC("icDoNotRecord", "://donotrecord.png", size); }

    static QIcon icErase() { return foundIC("icErase", "://eraser.png"); }
    static QIcon icEuro() { return foundIC("icEuro", "://euro.ico", QSize(40,40)); }
    static QIcon icEuroCount() { return foundIC("icEuroCount", "://EuroCount.png", QSize(40,40)); }

    static QIcon icFamily() { return foundIC("icFamily", "://family.png", QSize(30,30)); }
    static QIcon icFerme(QSize size = QSize(30,30)) { return foundIC("icFermeRelache", "://shutdown.png", size); }
    static QIcon icFermeAppuye(QSize size = QSize(30,30)) { return foundIC("icFermeAppuye", "://shutdowndown.png", size); }
    static QIcon icFlecheHaut() { return foundIC("icFlHaut", "://up-arrow-icon.png", QSize(30,30)); }
    static QIcon icFont() { return foundIC("icFont", "://font.png", QSize(30,30)); }
    static QIcon icFontmore() { return foundIC("icFontmore", "://fontsizemore.ico", QSize(30,30)); }
    static QIcon icFontless() { return foundIC("icFontless", "://fontsizeless.ico", QSize(30,30)); }
    static QIcon icFontbold() { return foundIC("icFontbold", "://fontbold.ico", QSize(30,30)); }
    static QIcon icFontitalic() { return foundIC("icFontitalic", "://fontitalic.ico", QSize(30,30)); }
    static QIcon icFontunderline() { return foundIC("icFontunderline", "://fontunderline.ico", QSize(30,30)); }
    static QIcon icFontnormal() { return foundIC("icFontnormal", "://fontnormal.ico", QSize(30,30)); }

    static QIcon icHelp() { return foundIC("icHelp", "://help.png", QSize(30,30)); }

    static QIcon icImportant() { return foundIC("icImportant", "://redbutton.png", QSize(25,25)); }
    static QIcon icImprimer(QSize size = QSize(25,25)) { return foundIC("icImprimer", pxImprimer(), size); }
    static QIcon icInformation() { return foundIC("icInformation", pxInformation(), QSize(25,25)); }
    static QIcon icInternet() { return foundIC("icInternet", "://internet.png", QSize(25,25)); }

    static QIcon icListe() { return foundIC("icListe", pxListe()); }
    static QIcon icLoupe() { return foundIC("icLoupe", pxLoupe()); }
    static QIcon icLunettes() { return foundIC("icLunettes", "://LunettesRondes.png"); }

    static QIcon icMarteau() { return foundIC("icMarteau", "://wrench-screwdriver.png"); }
    static QIcon icMedoc() { return foundIC("icMedoc", "://medoc.png"); }
    static QIcon icMessage() { return foundIC("icMessage", "://3Dmessage.png", QSize(40,40)); }
    static QIcon icNetwork() { return foundIC("icNetwork", "://network.png", QSize(40,40)); }

    static QIcon icNull(QSize size = QSize()) { return foundIC("icNull", "", size); } //30

    static QIcon icOK(QSize size = QSize()) { return foundIC("icOK", "://start.png", size); }
    static QIcon icOKpushed(QSize size = QSize()) { return foundIC("icOKpushed", "://startpushed.png", size); }
    static QIcon icOups(QSize size = QSize(45,45)) { return foundIC("icOups", pxOups(), size); }

    static QIcon icPageApres() { return foundIC("icPageApres", "://button_blue_ffw.png"); }
    static QIcon icPageAvant() { return foundIC("icPageAvant", "://button_blue_rew.png"); }
    static QIcon icPageHome() { return foundIC("icPageHome", "://home.png"); }
    static QIcon icPageRefresh() { return foundIC("icPageRefresh", "://button_blue_refresh.png"); }
    static QIcon icPassword() { return foundIC("icPassword", "://password.ico"); }
    static QIcon icPhoto() { return foundIC("icPhoto", "://camera.png"); }
    static QIcon icPostit(QSize size = QSize()) { return foundIC("icPostit", "://Post-it.png", size); } //25
    static QIcon icPoubelle(QSize size = QSize()) { return foundIC("icPoubelle", pxPoubelle(), size); } //25

    static QIcon icQuestion() { return foundIC("icQuestion", pxQuestion(), QSize(25,25)); }
    static QIcon icQwant() { return foundIC("icQwant", pxQwant(), QSize(25,25)); }

    static QIcon icReception(QSize size = QSize()) { return foundIC("icReception", pxReception(), size); }
    static QIcon icRecopier(QSize size = QSize()) { return foundIC("icRecopier", "://copy_v2.png", size); }
    static QIcon icReinit(QSize size = QSize()) { return foundIC("icReinit", "://reset.png", size); } //40

    static QIcon icSauvegarder(QSize size = QSize()) { return foundIC("icSauvegarder", "://save.png", size); }
    static QIcon icSide() { return foundIC("icSide", "://right_left.png"); }
    static QIcon icSunglasses() { return foundIC("icSunglasses", "://Sunglasses.png"); }
    static QIcon icSupprimer() { return foundIC("icSmoking", "://Supprime.png", QSize(30,30)); }
    static QIcon icSmoking() { return foundIC("icSmoking", "://No_smoking.png"); }
    static QIcon icSortirDossier() { return foundIC("icSortirDossier", "://metadoc.png", QSize(25,25)); }
    static QIcon icStetho() { return foundIC("icStetho", "://stetho.png"); }

    static QIcon icTampon() { return foundIC("icTampon", "://stamp.png"); }
    static QIcon icText() { return foundIC("icText", "://text.png"); }
    static QIcon icTri() { return foundIC("icTri", "://tri.png"); }

    static QIcon icValide(QSize size = QSize()) { return foundIC("icValide", "://Valide2.jpg", size); } //25
    static QIcon icVitale(QSize size = QSize()) { return foundIC("icVitale", "://Vitale.jpg", size); }
    static QIcon icUpdate(QSize size = QSize()) { return foundIC("icUpdate", "://update.png", size); } //20
};


#endif // ICONS_H
