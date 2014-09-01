#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QStringListModel>
#include <QLineEdit>
#include <QTextEdit>
#include <QMdiSubWindow>
#include <QMdiArea>
#include <QPair>
#include "codeeditor.h"
#include <QHBoxLayout>
#include <QPalette>
#include "instruction.h"
#include <QVector>
#include <QMessageBox>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QSignalMapper>
#include "codeeditorwindow.h"
#include "fileloader.h"
#include "explorertreeitem.h"
#include <QFileDialog>



#include "memory.h"
#include "InstructionFuncs.h"

QString MainWindow::projectPath;
QString MainWindow::projectFileName;
QString MainWindow::projectTitle;
QString MainWindow::projectMainFile;
QStringList MainWindow::projectTextFiles;
QString MainWindow::projectDataFile;
QMap<QString, QString> MainWindow::projectConf;




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow){



    ui->setupUi(this);
    setCentralWidget(ui->dockCode);
    installEventFilter(this);



    ui->actionTileset_viewer->setEnabled(false);
    //ui->actionBitmap_Display->setEnabled(false);
    ui->actionPalette_Viewer->setEnabled(false);
    ui->actionReload_Tiles_Memory->setEnabled(false);
    ui->actionTile_loader->setEnabled(false);
    ui->actionSprite_Editor->setEnabled(false);

    QFontDatabase fontsDB;
    fontsDB.addApplicationFont(":/font/consolas.ttf");
    if(fontsDB.families().contains("Consolas")){
        editorFont = fontsDB.font("Consolas", "Normal", 12);
    }

    treeWidget = ui->treeFiles;

    memory = new Memory(this);

    memoryLoading = NULL;
    ui->tableMemory->setCurrentIndex(1);

    for (int i = 0; i < 32; i++){
        mainProcessorRegisters.append(0);
    }
    mainProcessorRegisters[28] = 0x10008000;
    mainProcessorRegisters[29] = 0x7FFFFFFC;

    regModel = new RegistersModel(&mainProcessorRegisters, this, ui->registersNaming, ui->registersBase);
    ui->tableMainRegisters->setModel(regModel);

    assem = NULL;
    assemblerInitialized = false;
    this->setMouseTracking(true);



    textModel = new MemoryModel(memory, this, TextSegment, ui->textAddressMode, ui->textMemoryMode, ui->textMemoryBase);
    dataModel = new MemoryModel(memory, this, DataSegment, ui->dataAddressMode, ui->dataMemoryMode, ui->dataMemoryBase);
    stackModel = new MemoryModel(memory, this, StackSegment, ui->stackAddressMode, ui->stackMemoryMode, ui->stackMemoryBase);
    heapModel = new MemoryModel(memory, this, HeapSegment, ui->heapAddressMode, ui->heapMemoryMode, ui->heapMemoryBase);

    ui->textTable->setModel(textModel);
    ui->dataTable->setModel(dataModel);
    ui->stackTable->setModel(stackModel);
    ui->heapTable->setModel(heapModel);

    ui->actionOpen_Project->trigger();

    ui->treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);
    refreshEditActions();
    QObject::connect(ui->treeFiles, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(projectExplorerMenuRequested(QPoint)));
    QObject::connect(ui->mdiAreaCode,SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(refreshEditActions()));
    QObject::connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(activeWindowCopy()));
    QObject::connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(activeWindowCut()));
    QObject::connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(activeWindowPaste()));
    QObject::connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(activeWindowUndo()));
    QObject::connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(activeWindowRedo()));

}


bool MainWindow::eventFilter(QObject *, QEvent *e){
    if (e->type() == QEvent::Show)
        addEditorWindow();
    //on_actionNew_triggered();
    return false;
}

void MainWindow::addEditorWindow()
{
    /*QMdiSubWindow *newWindow = new QMdiSubWindow(ui->mdiAreaCode);
    newWindow->setObjectName("newW");
    newWindow->setWindowTitle("Untitled");


    QWidget *newWidgets = new QWidget(newWindow);
    newWidgets->setObjectName("NW");
    QHBoxLayout *HL = new QHBoxLayout(newWidgets);
    CodeEditor *newCode = new CodeEditor(newWidgets);
    QTextEdit *linesCount = new QTextEdit(newWidgets);

    newCode->setObjectName("CodeE");
    newCode->setFont(editorFont);

    linesCount->setEnabled(false);
    linesCount->setMinimumSize(10, 10);
    linesCount->setMaximumSize(40, linesCount->maximumSize().height());
    linesCount->setText("0");
    newCode->setCounter(linesCount);

    HL->setObjectName("HL");
    HL->addWidget(linesCount);
    HL->addWidget(newCode);

    newWidgets->setLayout(HL);

    newWindow->setWidget(newWidgets);

    newWindow->setAttribute(Qt::WA_DeleteOnClose, 1);*/
    CodeEditorWindow *editorWindow = new CodeEditorWindow(ui->mdiAreaCode, editorFont);
    ui->mdiAreaCode->addSubWindow(editorWindow);
    //newWidgets->showMaximized();
}

void MainWindow::addEditorWindow(QString file)
{
    CodeEditorWindow *editorWindow = new CodeEditorWindow(ui->mdiAreaCode, editorFont);
    if(editorWindow->openFile(file)){
        ui->mdiAreaCode->addSubWindow(editorWindow);
    }else{
        delete editorWindow;
        QMessageBox::critical(this, "Error", "Failed to open the file " + file);
    }

}

void MainWindow::createDataFile(QString file)
{

}

void MainWindow::creatTextFile(QString file)
{

}

void MainWindow::addDataFile(QString file)
{

}

void MainWindow::addTextFile(QString file)
{

}

void MainWindow::addResourceFile(QString file)
{

}

void MainWindow::removeDataFile(QString file)
{

}

void MainWindow::removeTextFile(QString file)
{

}

void MainWindow::removeResourceFile(QString file)
{

}

QString MainWindow::getProjectPath()
{
    return MainWindow::projectPath;
}

void MainWindow::setProjectPath(QString path)
{
    MainWindow::projectPath = path;
}

QString MainWindow::getProjectFileName(){
    return MainWindow::projectFileName;
}

QString MainWindow::getProjectTitle(){
    return MainWindow::projectTitle;
}

QString MainWindow::getProjectMainFile(){
    return MainWindow::projectMainFile;
}

QStringList MainWindow::getProjectTextFiles(){
    return MainWindow::projectTextFiles;
}

QString MainWindow::getProjectDataFile(){
    return MainWindow::projectDataFile;
}

QString MainWindow::getProjectConf(QString key){
    if (MainWindow::projectConf.contains(key))
        return MainWindow::projectConf[key];
    else
        return "";
}

bool MainWindow::isLittleEndian(){
    if (MainWindow::projectConf.contains("Endianness"))
        if (MainWindow::projectConf["Endianness"].toLower() == "big")
            return false;
    return true;
}

bool MainWindow::isGFXEnabled(){
    if (MainWindow::projectConf.contains("EnableGFX"))
        if (MainWindow::projectConf["EnableGFX"].toLower() == "true")
            return true;
    return false;
}

int MainWindow::getTileMapWidth(){
    if (MainWindow::projectConf.contains("TileMapWidth")){
        if (MainWindow::projectConf["TileMapWidth"].trimmed() == "3")
            return 3;
        else if (MainWindow::projectConf["TileMapWidth"].trimmed() == "2")
            return 2;
        else if (MainWindow::projectConf["TileMapWidth"].trimmed() == "2")
            return 1;
    }
    return 4;
}

int MainWindow::getTileMapHeight(){
    if (MainWindow::projectConf.contains("TileMapHeight")){
        if (MainWindow::projectConf["TileMapHeight"].trimmed() == "3")
            return 3;
        else if (MainWindow::projectConf["TileMapHeight"].trimmed() == "2")
            return 2;
        else if (MainWindow::projectConf["TileMapHeight"].trimmed() == "2")
            return 1;
    }
    return 4;
}



MainWindow::~MainWindow(){
    //delete memory;
    if(assemblerInitialized)
        delete assem;
    if (engine)
        delete engine;
    delete ui;
}

void MainWindow::projectExplorerMenuRequested(QPoint loc){
    // QModelIndex index = ui->treeFiles->indexAt(loc);
    QTreeWidgetItem *itm = ui->treeFiles->itemAt(loc);
    QMenu *menu=new QMenu(this);
    if (itm && itm->parent() && (itm->parent()->text(0) == "Data" || itm->parent()->text(0) == "Text")){
        //qDebug() << itm->text(0);
        QAction *openAction = new QAction(this);
        QSignalMapper *mapper = new QSignalMapper(this);
        mapper->setMapping(openAction, (QObject *) itm);
        QObject::connect(openAction, SIGNAL(triggered()), mapper, SLOT(map()));
        QObject::connect(mapper, SIGNAL(mapped(QObject*)), this, SLOT(openTreeItem(QObject*)));
        openAction->setText("Open");
        QAction *removeAction = new QAction(this);
        removeAction->setText("Remove");
        menu->addAction(openAction);
        menu->addAction(removeAction);
    }else{
        menu->addAction(ui->actionNew);
        menu->addAction(ui->actionOpen);
    }


    menu->popup(ui->treeFiles->viewport()->mapToGlobal(loc));
}

void MainWindow::on_actionSimulate_triggered(){
    // qDebug() << "Simulating..";
    engine = new TileEngine(0, QPoint(0,0), QSize(512,384), memory);
    memory->setTileEngine(engine);
    if(ui->actionEnable_Graphics_Engine->isChecked()){
        engine->show();
    }

    if (assemblerInitialized){
        assem->simulate();
        mainProcessorRegisters = *assem->registers;


        regModel = new RegistersModel(&mainProcessorRegisters, this, ui->registersNaming, ui->registersBase);
        ui->tableMainRegisters->setModel(regModel);

        textModel = new MemoryModel(memory, this, TextSegment, ui->textAddressMode, ui->textMemoryMode, ui->textMemoryBase);
        dataModel = new MemoryModel(memory, this, DataSegment, ui->dataAddressMode, ui->dataMemoryMode, ui->dataMemoryBase);
        stackModel = new MemoryModel(memory, this, StackSegment, ui->stackAddressMode, ui->stackMemoryMode, ui->stackMemoryBase);
        heapModel = new MemoryModel(memory, this, HeapSegment, ui->heapAddressMode, ui->heapMemoryMode, ui->heapMemoryBase);

        ui->textTable->setModel(textModel);
        ui->dataTable->setModel(dataModel);
        ui->stackTable->setModel(stackModel);
        ui->heapTable->setModel(heapModel);
        // qDebug() << "Simulated.";
    }

}

void MainWindow::on_actionNew_triggered(){

    FileLoader *loader = new FileLoader(this, CREATE_FILE);
    loader->show();
    addEditorWindow();

}

void MainWindow::printS(){

}

void MainWindow::on_actionAssemble_triggered(){
    // qDebug() << "Assembling..";
    if (ui->mdiAreaCode->currentSubWindow())
    {
        QWidget *W  = ui->mdiAreaCode->currentSubWindow()->findChild <QWidget *> ("NW");
        if (W){
            CodeEditor  *E = W->findChild  <CodeEditor*> ("CodeE");
            if (E){
                //qDebug() << E->toPlainText();
                QStringList instrs = E->toPlainText().split("\n");
                if(assemblerInitialized){
                    delete assem;
                    if (memory){
                        Memory *tempMemory = memory;
                        memory = new Memory(this);
                        delete tempMemory;
                    }

                }
                assem = new Assembler(&instrs, memory, &mainProcessorRegisters);
                assemblerInitialized = true;
            }else
                QMessageBox::critical(this, "Error", "Error 1");
        }else
            QMessageBox::critical(this, "Error", "Error 2");

    }else
        QMessageBox::critical(this, "Error", "Error 3");
    //qDebug() << "Assembled.";
}

void MainWindow::on_actionClose_triggered(){
    ui->mdiAreaCode->closeAllSubWindows();
}

void MainWindow::on_actionTileset_viewer_triggered(){
    tileSetViewer = new TileSetViewer(this, memory);
    tileSetViewer->show();
}

void MainWindow::on_actionPalette_Viewer_triggered(){
    paletteViewer = new PaletteViewer(this, memory);
    paletteViewer->show();
}

void MainWindow::on_actionTile_loader_triggered(){
    //tileLoader = new TileLoader(this, memory);
    //tileLoader->show();
}

void MainWindow::on_actionEnable_Graphics_Engine_triggered()
{
    if(ui->actionEnable_Graphics_Engine->isChecked()){
        /*if(memoryLoading == NULL || !memoryLoading->isInit()){
            if (memoryLoading == NULL)
                memoryLoading = new MemoryLoading(0, this->memory);
            memoryLoading->setModal(true);
            memoryLoading->show();
        }*/
        ui->actionEnable_Graphics_Engine->setText("Disable Graphics Engine");
        ui->actionEnable_Graphics_Engine->setToolTip("Disable Graphics Engine");
        ui->actionTileset_viewer->setEnabled(true);
        //ui->actionBitmap_Display->setEnabled(true);
        ui->actionPalette_Viewer->setEnabled(true);
        ui->actionReload_Tiles_Memory->setEnabled(true);
        //ui->actionTile_loader->setEnabled(true);
        ui->actionSprite_Editor->setEnabled(true);
    }else{
        ui->actionEnable_Graphics_Engine->setText("Enable Graphics Engine");
        ui->actionEnable_Graphics_Engine->setToolTip("Enable Graphics Engine");
        ui->actionTileset_viewer->setEnabled(false);
        //ui->actionBitmap_Display->setEnabled(false);
        ui->actionPalette_Viewer->setEnabled(false);
        ui->actionReload_Tiles_Memory->setEnabled(false);
        ui->actionTile_loader->setEnabled(false);
        ui->actionSprite_Editor->setEnabled(false);
    }
}

void MainWindow::on_actionReload_Tiles_Memory_triggered()
{

    if (memoryLoading != NULL){
        if (memoryLoading->isVisible()){
            memoryLoading->show();
            return;
        }
        delete memoryLoading;
    }

    memoryLoading = new MemoryLoading(0, this->memory);
    memoryLoading->setModal(true);
    memoryLoading->show();
}

void MainWindow::on_actionAssemble_and_Simulate_triggered()
{
    ui->actionAssemble->trigger();
    ui->actionSimulate->trigger();
}

void MainWindow::on_actionSprite_Editor_triggered()
{
    spriteViewer = new SpriteViewer(this, memory);
    spriteViewer->show();
}

void MainWindow::on_actionOpen_Project_triggered()
{
    QString tempProjectFileName = QFileDialog::getOpenFileName(this, "Open Project", QDir::currentPath() /* + "/projects/"*/, "Mirage Project File(*.mpro)");
    if (tempProjectFileName.trimmed() == "")
        return;
    QString tempProjectPath = QFileInfo(tempProjectFileName).absolutePath() + "/";
    tempProjectFileName = QFileInfo(tempProjectFileName).fileName();
    QFile tempProjectFile(tempProjectPath + tempProjectFileName);
    QString tempCurrentProjectString = "";
    QString line;
    QTextStream stream(&tempProjectFile);

    if (tempProjectFile.open(QIODevice::ReadOnly | QIODevice::ReadOnly)){
        tempCurrentProjectString = "";
        while (!stream.atEnd()){
            line = stream.readLine();
            tempCurrentProjectString.append(line + '\n');
        }
        //qDebug() << currentProjectString;
        tempProjectFile.close();
        if (parseProjectXML(tempProjectFile)){
            if(validateProjectFiles(false)){
                MainWindow::projectFileName = tempProjectFileName;
                projectFile.setFileName(tempProjectFile.fileName());
                MainWindow::projectPath = tempProjectPath;
                currentProjectString = tempCurrentProjectString;
                loadProjectTree();
            }

        }
    }else{
        QMessageBox::critical(this, "Error", QString("Failed to open project file") + QString("\n ") + tempProjectFile.errorString());
        qDebug() << "Failed to open!";
    }

}



bool MainWindow::parseProjectXML(QFile &data){

    if(!data.open( QIODevice::ReadOnly | QIODevice::Text ) ){
        QMessageBox::critical(this, "Error", QString("Failed to open project file") + QString("\n ") + data.errorString());
        qDebug() << "Failed to open!";
    }else{

        QDomDocument domDocument;
        if(!domDocument.setContent(&data)){
            qDebug() << "Cannot set content";
            return false;
        }

        data.close();

        QString tempProjectTitle;
        QString tempProjectMainFile;
        QStringList tempProjectTextFiles;
        QString tempProjectDataFile;
        QMap<QString, QString> tempProjectConf;

        QDomElement domElement = domDocument.documentElement();
        if (domElement.tagName().trimmed() != "MIPSProject"){
            QMessageBox::critical(this, "Error", "Invalid project file");
            return false;
        }
        QDomNode child = domElement.firstChild();
        while(!child.isNull()) {
            QDomElement e = child.toElement();
            if (e.tagName().trimmed() == "ProjectTitle"){
                tempProjectTitle = e.toElement().text().trimmed();
            }else if (e.tagName().trimmed() == "TextSegment"){
                QDomNodeList textL = e.childNodes();
                tempProjectTextFiles.clear();
                for (int i = 0; i < textL.size(); i++){
                    QString tempTag = textL.at(i).toElement().tagName().trimmed();
                    QString tempTagValue = textL.at(i).toElement().text().trimmed();

                    if (tempTag == "MainFile"){
                        // qDebug() << "Main: " << tempTagValue;
                        tempProjectMainFile = tempTagValue;
                        tempProjectTextFiles.append(tempTagValue);
                    }else if (tempTag == "File"){
                        tempProjectTextFiles.append(tempTagValue);
                    }
                }
            }else if (e.tagName().trimmed() == "DataSegment"){
                tempProjectDataFile = e.firstChild().toElement().text().trimmed();
            }else if (e.tagName().trimmed() == "Configure"){
                QDomNodeList confL = e.childNodes();
                tempProjectConf.clear();
                for (int i = 0; i < confL.size(); i++){
                    tempProjectConf[confL.at(i).toElement().tagName().trimmed()] = confL.at(i).toElement().text().trimmed();
                     }
            }else{
                QMessageBox::critical(this, "Error", "Unidentified element " + e.tagName().trimmed() + " in the project file");
                return false;
            }
            child = child.nextSibling();

        }
        MainWindow::projectTitle =  tempProjectTitle;
        MainWindow::projectMainFile =  tempProjectMainFile;
        MainWindow::projectTextFiles = tempProjectTextFiles;
        MainWindow::projectDataFile = tempProjectDataFile;
        MainWindow::projectConf = tempProjectConf;
        return true;
    }

    return false;

}

void MainWindow::loadProjectTree()
{
    ExplorerTreeItem *projectItem = new ExplorerTreeItem(treeWidget);
    QFont currentProjectFont = projectItem->font(0);
    currentProjectFont.setBold(true);
    projectItem->setText(0, MainWindow::projectTitle);
    projectItem->setFont(0, currentProjectFont);
    treeWidget->addTopLevelItem(projectItem);

    ExplorerTreeItem *textSegmentParent = new ExplorerTreeItem(projectItem);
    textSegmentParent->setText(0, "Text");
    projectItem->addChild(textSegmentParent);
    foreach(const QString &textFile, MainWindow::projectTextFiles){
        QTreeWidgetItem *textFileItem = new QTreeWidgetItem(textSegmentParent);
        textFileItem->setText(0, textFile);
        textFileItem->setIcon(0, QIcon(":/icons/explorer/icons/explorer/textsegmentFileIcon.png"));
        if (textFile == MainWindow::projectMainFile){
            QFont currentItemFont = textFileItem->font(0);
            currentItemFont.setItalic(true);
            textFileItem->setFont(0, currentItemFont);
        }
        textSegmentParent->addChild(textFileItem);
    }

    ExplorerTreeItem *dataSegmentParent = new ExplorerTreeItem(projectItem);
    dataSegmentParent->setText(0, "Data");
    projectItem->addChild(dataSegmentParent);
    ExplorerTreeItem *dataFileItem = new ExplorerTreeItem(dataSegmentParent);
    dataFileItem->setText(0, MainWindow::projectDataFile);
    dataFileItem->setIcon(0, QIcon(":/icons/explorer/icons/explorer/dataFileIcon.png"));
    dataSegmentParent->addChild(dataFileItem);

    projectItem->setExpanded(true);
    textSegmentParent->setExpanded(true);
    dataSegmentParent->setExpanded(true);
    //textSegmentParent->setIcon(0, QIcon(":/icons/explorer/icons/explorer/folderIconOpened.png"));
    //dataSegmentParent->setIcon(0, QIcon(":/icons/explorer/icons/explorer/folderIconOpened.png"));
}

bool MainWindow::validateProjectFiles(bool forceAll = true)
{
    foreach(const QString &textFile, MainWindow::projectTextFiles){
        QFile testFile(textFile);
        if (!testFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::critical(this, "Error", QString("Failed to load the file ") + textFile + QString("\n ") + testFile.errorString());
            if(!forceAll)
                return false;
        }else{
            //qDebug() << "Opened " << textFile;
            testFile.close();
        }
    }
    QFile testFile(MainWindow::projectDataFile);
    if (!testFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(this, "Error", QString("Failed to load the file ") + projectDataFile + QString("\n ") + testFile.errorString());
        if(!forceAll)
            return false;
    }else{
        //qDebug() << "Opened " << projectDataFile;
        testFile.close();
    }


    return true;
}

void MainWindow::on_actionInput_triggered()
{
    inputManager = new InputManager(this, memory);
    inputManager->show();
}

void MainWindow::on_treeFiles_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if(item->parent()){
        QString parentItemText = item->parent()->text(0).trimmed();
        if (parentItemText == "Text" || parentItemText == "Data"){
            QString fileName = item->text(0);

            foreach(QMdiSubWindow *window, ui->mdiAreaCode->subWindowList()){
                if(((CodeEditorWindow*) window)->getFilePath() == fileName){
                    ui->mdiAreaCode->setActiveSubWindow(window);
                    //qDebug() << ((CodeEditorWindow*) window)->getFilePath();
                    return;
                }
            }
            addEditorWindow(fileName);
        }
    }

}

void MainWindow::on_treeFiles_itemExpanded(QTreeWidgetItem *item){
    if (item->text(0) == "Text" || item->text(0) == "Data")
        item->setIcon(0, QIcon(":/icons/explorer/icons/explorer/folderIconOpened.png"));

}

void MainWindow::on_treeFiles_itemCollapsed(QTreeWidgetItem *item){
    if (item->text(0) == "Text" || item->text(0) == "Data")
        item->setIcon(0, QIcon(":/icons/explorer/icons/explorer/folderIconClosed.png"));
}

void MainWindow::on_actionViewProjectManager_triggered(){
    if(ui->actionViewProjectManager->isChecked())
        ui->dockProject->setVisible(true);
    else
        ui->dockProject->setVisible(false);

}

void MainWindow::on_actionViewAssemble_triggered(){
    if(ui->actionViewAssemble->isChecked())
        ui->dockAsm->setVisible(true);
    else
        ui->dockAsm->setVisible(false);
}

void MainWindow::on_actionViewCodeEditor_triggered(){
    if(ui->actionViewCodeEditor->isChecked())
        ui->dockCode->setVisible(true);
    else
        ui->dockCode->setVisible(false);
}

void MainWindow::on_actionViewMemory_triggered(){
    if(ui->actionViewMemory->isChecked())
        ui->dockMemory->setVisible(true);
    else
        ui->dockMemory->setVisible(false);
}

void MainWindow::on_actionOpen_triggered(){
    FileLoader *loader = new FileLoader(this, ADD_FILE);
    loader->show();
}

void MainWindow::openTreeItem(QObject *itm){
    // qDebug() << "Triggered" << ((QTreeWidgetItem *)itm)->text(0);
    this->on_treeFiles_itemDoubleClicked((QTreeWidgetItem *)itm, 0);
}

void MainWindow::activeWindowCopy(){
    CodeEditorWindow *activeWindow = dynamic_cast<CodeEditorWindow *>(ui->mdiAreaCode->activeSubWindow());
    if (activeWindow){
        activeWindow->copy();
    }
}

void MainWindow::activeWindowCut(){
    CodeEditorWindow *activeWindow = dynamic_cast<CodeEditorWindow *>(ui->mdiAreaCode->activeSubWindow());
    if (activeWindow){
        activeWindow->cut();
    }else
        qDebug() << "No active window";
}

void MainWindow::activeWindowPaste(){
    CodeEditorWindow *activeWindow = dynamic_cast<CodeEditorWindow *>(ui->mdiAreaCode->activeSubWindow());
    if (activeWindow){
        activeWindow->paste();
    }
}

void MainWindow::activeWindowUndo(){
    CodeEditorWindow *activeWindow = dynamic_cast<CodeEditorWindow *>(ui->mdiAreaCode->activeSubWindow());
    if (activeWindow){
        activeWindow->undo();
    }
}

void MainWindow::activeWindowRedo(){
    CodeEditorWindow *activeWindow = dynamic_cast<CodeEditorWindow *>(ui->mdiAreaCode->activeSubWindow());
    if (activeWindow){
        activeWindow->redo();
    }
}

void MainWindow::refreshActions(){

}

void MainWindow::refreshEditActions(){
    CodeEditorWindow *activeWindow = dynamic_cast<CodeEditorWindow *>(ui->mdiAreaCode->activeSubWindow());
    if (activeWindow){
        ui->actionCopy->setEnabled(true);
        ui->actionCut->setEnabled(true);
        ui->actionPaste->setEnabled(true);
        ui->actionUndo->setEnabled(true);
        ui->actionRedo->setEnabled(true);
    }else{
        ui->actionCopy->setEnabled(false);
        ui->actionCut->setEnabled(false);
        ui->actionPaste->setEnabled(false);
        ui->actionUndo->setEnabled(false);
        ui->actionRedo->setEnabled(false);
    }
}
