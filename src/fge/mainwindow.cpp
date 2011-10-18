#include "mainwindow.h"

#include <iostream>

#include <QtGui>
#include <Qsci/qsciscintilla.h>

#include "fglexer.h"
#include "consolewidget.h"
#include "redirect.h"
#include "exporter.h"

#include "ui_mainwindow.h"
#include "ui_exportdialog.h"


MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
,mUniverse(NULL)
,mSimulationTimer(NULL)
,mSimulationMode(SM_PAUSED)
,mPreviousMode(SM_PAUSED)
,mTimeMultiplier(1)
,mTime(0)
,mActiveScript(NULL)
{
	Ui::MainWindow ui;
	ui.setupUi(this);

	mConsoleWidget = findChild<ConsoleWidget*>("consolewidget");
	mFGView = findChild<FGView*>("fgview");
	mEditors = findChild<QTabWidget*>("editors");
	connect(mConsoleWidget, SIGNAL(emitCommand(QString)), this, SLOT(runScript(QString)));

	QTimer::singleShot(1, this, SLOT(redirectStreams()));

	// create action groups..

	QActionGroup* drawModeGroup = new QActionGroup(this);
	drawModeGroup->addAction(ui.actionSetDrawSmooth);
	drawModeGroup->addAction(ui.actionSetDrawFlat);
	drawModeGroup->addAction(ui.actionSetDrawWire);
	drawModeGroup->addAction(ui.actionSetDrawPoints);
	drawModeGroup->addAction(ui.actionSetDrawTextured);
	drawModeGroup->addAction(ui.actionSetDrawPhong);
	ui.actionSetDrawPhong->setChecked(true);

	QActionGroup* subdivModeGroup = new QActionGroup(this);
	subdivModeGroup->addAction(ui.actionSetSubdivs0);
	subdivModeGroup->addAction(ui.actionSetSubdivs1);
	subdivModeGroup->addAction(ui.actionSetSubdivs2);
	subdivModeGroup->addAction(ui.actionSetSubdivs3);
	ui.actionSetSubdivs1->setChecked(true);

	QActionGroup* colourModeGroup = new QActionGroup(this);
	colourModeGroup->addAction(ui.actionSetColourNone);
	colourModeGroup->addAction(ui.actionSetColourVertex);
	ui.actionSetColourVertex->setChecked(true);

	// load a script
	newEditor(new QFile("../scripts/ben/simple.lua"));
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About fugu"),
			tr("<p><b>fugu</b> is a code-oriented form generation tool.</p>"));
}

void MainWindow::newFile()
{
	// editor->clear();
	newEditor();
}

void MainWindow::open()
{
	openFile();
}

void MainWindow::save(){
	// Get the current opened file..
	QWidget* qw = mEditors->currentWidget();
	if (qw==NULL){
		QMessageBox::critical(this, tr("Application"), tr("No code to save!"));
		return;
	}
	else {
		if (mFileNames.contains(qw)){
			QString fileName = mFileNames[qw];
			if (fileName.isEmpty()){
				saveAs();
			}
			else {
				saveFile(static_cast<QsciScintilla*>(qw),fileName);
			}
		}
	}
}

void MainWindow::saveAs(){
	QWidget* qw = mEditors->currentWidget();
	if (qw==NULL){
		QMessageBox::critical(this, tr("Application"), tr("There is no open file to save!"));
		return;
	}
	else {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save As..."), "", "fugu script (*.lua)");
		if (fileName.isEmpty())
			return;

		saveFile(static_cast<QsciScintilla*>(qw),fileName);
		// reset filename
		mFileNames[qw] = fileName;
	}
}

void MainWindow::closeFile(){
	QWidget* qw = mEditors->currentWidget();
	if (qw==NULL){
		QMessageBox::critical(this, tr("Application"), tr("No selected file to close!"));
		return;
	}
	else {
		// TODO: check if we want to save...

		// Close file and remove active
		if (mFileNames.contains(qw)){
			mFileNames.remove(qw);
		}

		if (qw==mActiveScript)
		{
			unload();
			mActiveScript = NULL;
		}

		mEditors->removeTab(mEditors->currentIndex());
		delete qw;
	}
}

void MainWindow::quit(){
	QApplication::exit();
}

void MainWindow::load(){
	if (mActiveScript==NULL){
		QMessageBox::warning(this, tr("No Active Script"), tr("There must be an active script in order to run a simulation."));
		return;
	}
	else {
		std::cout << "Loading universe\n";
		if (mUniverse!=NULL){
			// Clean up the old universe
			mFGView->unsetUniverse();
			delete mUniverse;
			mUniverse = NULL;
			// TODO: make sure old slider values carry over into new state

			// mFGView->repaint();
		}

		try {
			// Create a new universe
			mUniverse = new fg::Universe();

			// add the search paths
			QString filename = mFileNames[mActiveScript];
			QFileInfo info = QFileInfo(filename);

			// QFile file(filename);
			QDir dir = info.dir();
			// dir.makeAbsolute();
			mUniverse->addScriptDirectory((dir.absolutePath() + "/?.lua").toStdString());
			mUniverse->addScriptDirectory("../scripts/?.lua");
			// mUniverse->addScriptDirectory("./?.lua");
			// mUniverse->loadScript("ben/aorta");
			// chop off .lua suffix

			QString filebase = info.completeBaseName();
			if (filebase.endsWith(".lua")){
				filebase.truncate(filebase.length()-4);
			}
			mUniverse->loadScript(filebase.toStdString());

			mFGView->setUniverse(mUniverse);
			// mFGView->repaint();
		}
		catch (std::runtime_error& e){
			mSimulationMode = SM_ERROR;
			std::cerr << "ERROR: " << e.what() << "\n";
			if (mUniverse!=NULL){
				delete mUniverse;
				mUniverse = NULL;
			}
		}
	}
}

void MainWindow::unload(){
	if (mUniverse!=NULL){
		if (mSimulationTimer) mSimulationTimer->stop();
		findChild<QAction*>("actionPlay")->setChecked(false);
		mFGView->unsetUniverse();
		delete mUniverse;
		mUniverse = NULL;
	}
}

void MainWindow::togglePlay(bool doplay){
	if (doplay) play();
	else pause();
}

void MainWindow::play(){
	if (mSimulationMode==SM_ERROR) {
		// don't change mode
	}
	else {
		if (mSimulationMode != SM_PLAYING){
			// start the timer
			if (mSimulationTimer==NULL){
				mSimulationTimer = new QTimer(this);
				connect(mSimulationTimer, SIGNAL(timeout()), this, SLOT(simulateOneStep()));
			}
			mSimulationTimer->start(10);
			mSimulationMode = SM_PLAYING;
		}
	}
}

void MainWindow::pause(){
	if (mSimulationMode==SM_ERROR) {
		// don't change mode
	}
	else {
		if (mSimulationTimer!=NULL){
			mSimulationTimer->stop();
		}
		mSimulationMode = SM_PAUSED;
		findChild<QAction*>("actionPlay")->setChecked(false);
	}

}

void MainWindow::step(){
	if (mSimulationMode==SM_ERROR) {
		// don't change mode
	}
	else {
		if (mSimulationTimer!=NULL){
			mSimulationTimer->stop();
		}
		mSimulationMode = SM_STEPPING;
		simulateOneStep();
		mSimulationMode = SM_PAUSED;
		findChild<QAction*>("actionPlay")->setChecked(false);
	}
}

void MainWindow::reload(){
	if (mSimulationMode==SM_ERROR) {
		mPreviousMode = SM_PAUSED;
	}
	else {
		mPreviousMode = mSimulationMode;
	}

	mSimulationMode = SM_RELOADING;
	load();

}

void MainWindow::simulateOneStep(){
	if (mUniverse!=NULL){
		try {
			mUniverse->update(0.01);
		}
		catch (std::runtime_error& e){
			std::cerr << "ERROR: " << e.what() << "\n";
			unload();
			mSimulationMode = SM_ERROR;
		}

		mFGView->update();
	}
}

void MainWindow::simulateOneStepNoViewUpdate(){
	if (mUniverse!=NULL){
		try {
			mUniverse->update(0.01);
		}
		catch (std::runtime_error& e){
			std::cerr << "ERROR: " << e.what() << "\n";
			unload();
			mSimulationMode = SM_ERROR;
		}
	}
}

void MainWindow::undo(){
	QWidget* qw = mEditors->currentWidget();
	if (qw!=NULL){
		static_cast<QsciScintilla*>(qw)->undo();
	}
}

void MainWindow::redo(){
	QWidget* qw = mEditors->currentWidget();
	if (qw!=NULL){
		static_cast<QsciScintilla*>(qw)->redo();
	}
}

void MainWindow::runScript(QString code){
	// std::cout << "Command: \"" << code.toStdString() << "\"\n";

	if (mUniverse and mSimulationTimer!=NULL and mSimulationTimer->isActive()){
		mSimulationTimer->stop();
		mUniverse->runScript(code.toStdString());
		mSimulationTimer->start(mSimulationTimer->interval());
	}
	else if (mSimulationMode!=SM_ERROR and mUniverse!=NULL){
		mUniverse->runScript(code.toStdString());
	}
	else {
		std::cerr << "Script must be active and universe successfully loaded to run a command.\n";
	}
}

void MainWindow::redirectStreams(){
	StdOutRedirector* s = redirectConsoleOutput(this);
	if (s){
		(void)connect( s, SIGNAL( caughtString( QString ) ),
				mConsoleWidget, SLOT( print( QString ) ));
		s->start();
	}
}

void MainWindow::makeCurrentScriptActive(){
	if (mActiveScript){
		int ind = mEditors->indexOf(mActiveScript);
		if (ind!=-1){
			// remove the '[' and ']'
			QFileInfo fi(mFileNames[mActiveScript]);
			mEditors->setTabText(ind,fi.fileName());
		}
	}

	mActiveScript = mEditors->currentWidget();
	if (mActiveScript){
		QFileInfo fi(mFileNames[mActiveScript]);
		// add '[' and ']' to the tab text
		mEditors->setTabText(mEditors->currentIndex(),QString("[") + fi.fileName() + "]");
		reload();
	}
}

void MainWindow::showLineNumbers(bool b){
	for(int i=0;i<mEditors->count();i++){
		QsciScintilla* editor = static_cast<QsciScintilla*>(mEditors->widget(i));
		if (!b){
			editor->setMarginWidth(1,0);
		}
		else {
			editor->setMarginWidth(1,QString("9999"));
		}
	}
}

void MainWindow::exportSimulation(){
	if (mUniverse==NULL){
		QMessageBox::critical(this, tr("Export"), tr("There is no simulation to export!"));
	}
	else {
		// stop the simulation
		// pause();
		findChild<QAction*>("actionPlay")->setChecked(false);

		mExportDialog = new QDialog(this);
		Ui::ExportDialog ui;
		ui.setupUi(mExportDialog);

		QLineEdit* qe = mExportDialog->findChild<QLineEdit*>("dirLineEdit");
		qe->setText(QDir("export").absolutePath());

		QPushButton* chooseDir = mExportDialog->findChild<QPushButton*>("chooseDirectoryPushButton");
		connect(chooseDir, SIGNAL(pressed()), this, SLOT(exportSimulationChooseDir()));

		QLineEdit* numFramesLE = mExportDialog->findChild<QLineEdit*>("numFramesLineEdit");

		if (mExportDialog->exec()){

			bool convert = false;
			int numFrames = numFramesLE->text().toInt(&convert);
			if (not convert or numFrames<=0){
				QMessageBox::critical(this, tr("Export"), tr("Number of frames is invalid or less than 1!"));
				return;
			}
			// else ...

			std::cout << "Saving to: " << QDir(qe->text()).absolutePath().toStdString() << "\n";

			QProgressDialog progress("Exporting...", "Abort", 0, numFrames, mExportDialog);
			progress.setWindowModality(Qt::WindowModal);
			progress.setAutoClose(true);
			progress.setMinimumDuration(1000); // 1 second..
			Exporter e = Exporter::ExportFrameToObj(mUniverse).dir(QDir(qe->text()));

			for (int i = 0; i < numFrames; i++) {
				progress.setValue(i);

				if (progress.wasCanceled())
					break;
				if (not e.run(i,numFrames)){
					QMessageBox::critical(this, tr("Export"), e.error());
					break;
				}
				simulateOneStepNoViewUpdate();
				if (mUniverse==NULL){
					QMessageBox::critical(this, tr("Export"), tr("The universe imploded, sorry."));
					break;
				}

			}
			progress.setValue(numFrames);

		}
	}
}

void MainWindow::exportSimulationChooseDir(){
	// QMessageBox::information(mExportDialog, tr("Choose Dir"), tr("You pressed choose dir dawg!"));
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Export Directory"),
			mExportDialog->findChild<QLineEdit*>("dirLineEdit")->text(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (not dir.isEmpty()){
		mExportDialog->findChild<QLineEdit*>("dirLineEdit")->setText(QDir(dir).absolutePath());
	}
}

void MainWindow::buildReference() // build the html reference
{
	// if universe doesn't exist, make one temporarily...
	bool tmpuni = false;
	if (mUniverse==NULL){
		tmpuni = true;
		try {
			mUniverse = new fg::Universe();
		}
		catch (std::runtime_error& e){
			mSimulationMode = SM_ERROR;
			std::cerr << "ERROR: " << e.what() << "\n";
			if (mUniverse!=NULL){
				delete mUniverse;
				mUniverse = NULL;
				return;
			}
		}
	}

	QFile file("reference.html");
	if (file.open(QIODevice::WriteOnly)){
		QTextStream out(&file);
		out << "<html><body>\n";
		typedef tuple<std::string,std::string,std::string> string3;
		std::list<string3> commandList = mUniverse->commandListByCategory();

		out << "</body></html>\n";
	}
	else {
		std::cerr << "Whoops, can't open reference.html.";
	}

	if (tmpuni){
		delete mUniverse;
		mUniverse = NULL;
	}
}

void MainWindow::openFile(const QString &path)
{
	QString fileName = path;
	if (fileName.isNull())
		fileName = QFileDialog::getOpenFileName(this,
				tr("Open File"), "", "fugu script (*.lua)");

	if (!fileName.isEmpty()) {

		// check if its already open...
		// if so, just switch to that file
		for(int i=0;i<mEditors->count();i++){
			if (mFileNames[mEditors->widget(i)]==fileName){
				mEditors->setCurrentIndex(i);
				return;
			}
		}
		// if not, open a new editor
		newEditor(new QFile(fileName));
	}
}

// Save the the editor's contents to the file fileName
bool MainWindow::saveFile(QsciScintilla* editor, QString fileName){
	// Save the currently active script
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly)) {
		QMessageBox::warning(this, tr("Application"),
							 tr("Cannot write file %1:\n%2.")
							 .arg(fileName)
							 .arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	out << editor->text();
	QApplication::restoreOverrideCursor();
	// statusBar()->showMessage(tr("File saved"), 2000);

	// rename tab label ...
	QFileInfo fi(file);
	if (mActiveScript==editor){
		mEditors->setTabText(mEditors->indexOf(editor),QString("[") + fi.fileName() + "]");
	}
	else {
		mEditors->setTabText(mEditors->indexOf(editor),fi.fileName());
	}
	return true;
}


void MainWindow::newEditor(QFile* file)
{
	QsciScintilla* editor = new QsciScintilla();
	editor->setLexer(new FGLexer());
	editor->setTabWidth(2);
	editor->setAutoIndent(true);

	editor->setMarginLineNumbers(1,true);
	bool b = findChild<QAction*>("actionShowLineNumbers")->isChecked();
	if (!b){
		editor->setMarginWidth(1,0);
	}
	else {
		editor->setMarginWidth(1,QString("9999"));
	}
	editor->setMarginWidth(2,8); // a little margin

	editor->setWrapMode(QsciScintilla::WrapCharacter);

	editor->setCaretForegroundColor(QColor("#959595"));
	editor->setMarginsBackgroundColor(QColor("#272727"));
	editor->setMarginsForegroundColor(QColor("#959595"));

	if (file==NULL){
		// Open a blank editor
		int t = mEditors->addTab(editor, "unnamed.lua");
		mFileNames.insert(editor,QString("unnamed.lua")); // add empty filename
	}
	else {
		if (file->open(QFile::ReadOnly | QFile::Text)){
			QFileInfo fi(*file);
			int t = mEditors->addTab(editor, fi.fileName());
			editor->setText(file->readAll());
			mFileNames.insert(editor,file->fileName());
		}
		delete file;
	}
	mEditors->setCurrentWidget(editor);

	if (!mActiveScript) makeCurrentScriptActive();
}
