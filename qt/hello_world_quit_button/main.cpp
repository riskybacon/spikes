#include <QtGui>

int main(int argc, char** argv)
{
   // Create a Qt application
   QApplication app(argc, argv);
   
   // Create a text editor widget
   QTextEdit*   textEdit   = new QTextEdit;

   // Create a quit button widget
   QPushButton* quitButton = new QPushButton("Quit");

   // Even though 'new' was used to allocate memory for the
   // widgets, they do not need to be freed using 'delete'.
   // Qt will handle the deletion for us.
  
   // Connect the quit button's clicked signal to the
   // applications quit() slot
   QObject::connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
   
   // Set up the layout for the widgets. The layout will be
   // handled automatically
   QVBoxLayout* layout = new QVBoxLayout;
   layout->addWidget(textEdit);
   layout->addWidget(quitButton);
   
   // Create a window and set its layour
   QWidget window;
   window.setLayout(layout);
   
   // Show the window
   window.show();
   
   // Start the application
   return app.exec();
}
