#include <QDebug>
#include <QFileDialog>

#include "core/headers/evenement.h"
#include "core/headers/projet.h"
#include "core/headers/programmation.h"
#include "core/headers/exportManager.h"

#include "headers/mainWindow.h"
#include "ui_mainWindow.h"
#include "headers/ajouterActivite.h"
#include "ui_ajouterActivite.h"
#include "headers/ajouterProjet.h"
#include "ui_ajouterProjet.h"
#include "headers/mainWindowProjet.h"
#include "ui_mainWindowProjet.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    jourSelectionne = QDate::currentDate();

    // Style calendar
    ui->calendrier->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    QTextCharFormat weekendFormat;
    weekendFormat.setForeground(QBrush(QColor(236,64,122,250), Qt::SolidPattern));
    ui->calendrier->setWeekdayTextFormat(Qt::Saturday, weekendFormat);
    ui->calendrier->setWeekdayTextFormat(Qt::Sunday, weekendFormat);
    QTableView *view = ui->calendrier->findChild<QTableView*>("qt_calendar_calendarview");
    if (view)
    {
        QPalette pal = view->palette();
        pal.setColor(QPalette::Highlight, QColor(26,128,182,250));
        view->setPalette(pal);
    }
    // Style week view
    QHeaderView* header = ui->vueHebdomadaire->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    this->updateListeActivites();
    this->updateListeProjets();
    this->updateVueHebdomadaire();

    connect(ui->ajouterActivite, SIGNAL (clicked()), this, SLOT (boutonajouterActivite()));
    connect(ui->ajouterProjet, SIGNAL (clicked()), this, SLOT (boutonajouterProjet()));
    connect(ui->exportProjet, SIGNAL (clicked()), this, SLOT (boutonExportProjet()));
    connect(ui->exportWeek, SIGNAL (clicked()), this, SLOT (boutonExportWeek()));
    connect(ui->calendrier, SIGNAL (clicked(QDate)), this, SLOT (updateJourSelectionne(QDate)));
    connect(ui->calendrier, SIGNAL (clicked(QDate)), this, SLOT (updateVueHebdomadaire()));
    connect(ui->listeProjets, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(projetClic(QListWidgetItem*)));
    connect(ui->listeProjets, SIGNAL(itemSelectionChanged()), this, SLOT(updateExportProjet()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*!
  Click capture event
 */
void MainWindow::boutonajouterActivite()
{
    ajouterActivite a;
    a.exec();
    this->updateListeActivites();
    this->updateVueHebdomadaire();
}


void MainWindow::boutonExportWeek()
{
    QString * selectedFilter = new QString();
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter la semaine",  QDir::current().absolutePath(), "XML(*.xml);;Texte brut(*.txt)", selectedFilter);
    Programmation& myProgrammation = Programmation::getInstance();
    QList<Evenement*>* events = myProgrammation.getWeekEvents(this->getSelectedMonday());
    if(*selectedFilter == "XML(*.xml)")
    {
        QFile * f = new QFile(fileName + ".xml");
        ExportManager exp(f, ExportManager::EXPORT_XML());
        exp.exportEvents(events);
        f->close();
        delete f;
    }
    else
    {
        QFile * f = new QFile(fileName+ ".txt");
        ExportManager exp(f, ExportManager::EXPORT_TXT());
        exp.exportEvents(events);
        f->close();
        delete f;
    }
    delete selectedFilter;
}


void MainWindow::boutonExportProjet()
{
    QString * selectedFilter = new QString();
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter un projet",  QDir::current().absolutePath(), "XML(*.xml);;Texte brut(*.txt)", selectedFilter);
    ProjetManager& myProjetManager = ProjetManager::getInstance();
    Projet* p = myProjetManager.getProjets()->value(ui->listeProjets->currentItem()->text().mid(4));
    if(*selectedFilter == "XML(*.xml)")
    {
        QFile * f = new QFile(fileName + ".xml");
        ExportManager exp(f, ExportManager::EXPORT_XML());
        exp.exportProjet(p);
        f->close();
        delete f;
    }
    else
    {
        QFile * f = new QFile(fileName+ ".txt");
        ExportManager exp(f, ExportManager::EXPORT_TXT());
        exp.exportProjet(p);
        f->close();
        delete f;
    }
    delete selectedFilter;
}

/*!
  Click capture event
 */
void MainWindow::boutonajouterProjet()
{
    ajouterProjet a;
    a.exec();
    this->updateListeProjets();
    this->updateVueHebdomadaire();
}

/*!
  Return the monday in the current week of calendar widget
   \return const QDate Date
 */
const QDate MainWindow::getSelectedMonday()
{
    return jourSelectionne.addDays(- jourSelectionne.dayOfWeek() + 1);
}

void MainWindow::updateListeActivites()
{
    while(ui->listeActivites->count()>0)
        ui->listeActivites->takeItem(0);

    QFont wantedFont;
    wantedFont.setPointSize(20);

    ActiviteManager& myActiviteManager = ActiviteManager::getInstance();
    foreach(Activite* activite, *myActiviteManager.getActivites())
    {
        QListWidgetItem * item = new  QListWidgetItem(activite->getNom());
        item->setFont(wantedFont);
        item->setIcon(QIcon(":/pink-document-open.svg"));
        ui->listeActivites->addItem(item);
    }
    ui->listeActivites->sortItems();
}

/*!
  Slot to save current date selected
   \param date unused
 */
void MainWindow::updateJourSelectionne(const QDate & date)
{
    Q_UNUSED(date);
    jourSelectionne = ui->calendrier->selectedDate();
}

/*!
  Launch the projet editor corresponding the the item clicked
   \param QListWidgetItem * projet the clicked item
 */
void MainWindow::projetClic(QListWidgetItem *projet)
{
    MainWindowProjet * a = new MainWindowProjet(this, projet->text().mid(4)); // Mid to delete the crayon icon
    a->setAttribute(Qt::WA_DeleteOnClose);
    a->show();
}

void MainWindow::updateExportProjet()
{
    ui->exportProjet->setDisabled(false);
}

void MainWindow::updateVueHebdomadaire() {

    //Nettoyage de l'affichage précédent
    ui->vueHebdomadaire->clearContents();
    ui->vueHebdomadaire->clearSpans();

    //Mise à jour des labels horizontaux
    QStringList listeJours(getSelectedMonday().toString("dddd d"));
    int i;
    for(i=1;i<7;i++){
        listeJours << getSelectedMonday().addDays(i).toString("dddd d");
    }
    ui->vueHebdomadaire->setHorizontalHeaderLabels(listeJours);

    //Mise à jour des labels Verticaux et style du tableau

    QStringList listeHeures;
    QTime h(8,0) ;
    ui->vueHebdomadaire->setStyleSheet("gridline-color: white;");
    int j;
    for(i=0; i<=72; i++){
        if(i%6 == 0){
            for(j=0;j<7;j++){
                QTableWidgetItem * case_heure = new QTableWidgetItem();
                case_heure->setBackgroundColor(QColor("#E8E8E8"));
                ui->vueHebdomadaire->setItem(i,j,case_heure);
            }
            listeHeures << h.toString("H:mm");
            h = h.addSecs(3600);
        }
        else
            listeHeures << QString("");
    }
    ui->vueHebdomadaire->verticalHeader()->setStyleSheet("QHeaderView::section {border : none;}");
    ui->vueHebdomadaire->setVerticalHeaderLabels(listeHeures);

    //Mise à jour des evenements de la semaine

    QFont policeTitre;
    policeTitre.setPointSize(12);

    Programmation& myProgrammation = Programmation::getInstance();
    foreach(Evenement* evenement, *myProgrammation.getWeekEvents(getSelectedMonday())){
        QTableWidgetItem * case_event = new QTableWidgetItem(evenement->getNom());
        case_event->setFont(policeTitre);
        case_event->setTextAlignment(Qt::AlignCenter);
        case_event->setBackgroundColor(evenement->getColor());
        case_event->setToolTip(evenement->getDesc());

        int colonne = evenement->getDebut()->date().daysTo(getSelectedMonday()) + 1;

        int ligne = (evenement->getDebut()->time().hour() - 8) * 6; //Heures
        ligne = ligne + evenement->getFin()->time().minute() / 10; //Minutes

        ui->vueHebdomadaire->removeCellWidget(ligne,-colonne + 1);
        ui->vueHebdomadaire->setItem(ligne,-colonne + 1,case_event);
        ui->vueHebdomadaire->setSpan(ligne,-colonne + 1,evenement->getDebut()->time().secsTo(evenement->getFin()->time()) / 600,1);

    }
}

void MainWindow::updateListeProjets()
{
    while(ui->listeProjets->count()>0)
        ui->listeProjets->takeItem(0);

    QFont wantedFont;
    wantedFont.setPointSize(20);

    ProjetManager& myProjetManager = ProjetManager::getInstance();
    foreach(Projet* projet, *myProjetManager.getProjets())
    {
        QListWidgetItem * item = new  QListWidgetItem(" ✎  " + projet->getNom());
        item->setFont(wantedFont);
        item->setToolTip("Cliquer pour éditer le projet");
        item->setIcon(QIcon(":/yellow-document-open.svg"));
        ui->listeProjets->addItem(item);
    }
    ui->listeProjets->sortItems();
    ui->exportProjet->setDisabled(true);
}
