#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QtOpenGL/QtOpenGL>

#include <osgQt/GraphicsWindowQt>
#include <iostream>

class InnerDialog : public QDialog
{
public:
    InnerDialog( QWidget* parent=0 )
    : QDialog(parent, Qt::CustomizeWindowHint|Qt::WindowTitleHint)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget( new QLabel(tr("Test label")) );
        layout->addWidget( new QCheckBox(tr("Test check box")) );
        layout->addWidget( new QPushButton(tr("Test push button")) );
        layout->addWidget( new QTextEdit(tr("Test text")) );
        setLayout( layout );
    }
};

class MyScene : public QGraphicsScene
{
public:
    MyScene() : QGraphicsScene()
    {
        setSceneRect( 0, 0, 800, 600 );
        
        InnerDialog* dialog = new InnerDialog;
        dialog->setWindowTitle( tr("Dialog") );
        dialog->setWindowOpacity( 0.75 );
        dialog->resize( 160, 100 );
        dialog->move( 20, 10 );
        
        QGraphicsProxyWidget* proxy = new QGraphicsProxyWidget;
        proxy->setWidget( dialog );
        proxy->setCacheMode( QGraphicsItem::ItemCoordinateCache );
        proxy->setZValue( 1e30 );
        addItem( proxy );
    }
    
    virtual void drawBackground( QPainter* painter, const QRectF& )
    {
        painter->beginNativePainting();
        
        glClearColor( 0.2, 0.2, 0.6, 1.0 );
        glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
        
        painter->endNativePainting();
    }
};

class MyGraphicsView : public QGraphicsView
{
public:
    MyGraphicsView() : QGraphicsView()
    {
        setRenderHints( QPainter::Antialiasing|QPainter::SmoothPixmapTransform );
        //setRenderHints( QPainter::SmoothPixmapTransform );
    }

protected:
    void resizeEvent(QResizeEvent *event)
    {
        if ( scene() )
            scene()->setSceneRect( QRect(QPoint(0, 0), event->size()) );
        QGraphicsView::resizeEvent( event );
    }
};

class ViewerWidget : public QWidget, public osgViewer::CompositeViewer
{
public:
    ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded) : QWidget()
    {
        setThreadingModel(threadingModel);

        QWidget* widget = addViewWidget( createCamera(0,0,100,100), osgDB::readNodeFile("cow.osgt") );
        _scene = new MyScene;
        
        MyGraphicsView* view = new MyGraphicsView;
        view->setViewport( widget );
        view->setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
        view->setScene( _scene );
        
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget( view );
        setLayout( layout );

        connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
        connect( &_timer, SIGNAL(timeout()), _scene, SLOT(update()) );
        _timer.start( 10 );
    }
    
    QWidget* addViewWidget( osg::Camera* camera, osg::Node* scene )
    {
        osgViewer::View* view = new osgViewer::View;
        view->setCamera( camera );
        addView( view );
        
        view->setSceneData( scene );
        view->addEventHandler( new osgViewer::StatsHandler );
        view->setCameraManipulator( new osgGA::TrackballManipulator );
        
        osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
        return gw ? gw->getGLWidget() : NULL;
    }
    
    osg::Camera* createCamera( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false )
    {
        osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->windowName = name;
        traits->windowDecoration = windowDecoration;
        traits->x = x;
        traits->y = y;
        traits->width = w;
        traits->height = h;
        traits->doubleBuffer = true;
        traits->alpha = ds->getMinimumNumAlphaBits();
        traits->stencil = ds->getMinimumNumStencilBits();
        traits->sampleBuffers = ds->getMultiSamples();
        traits->samples = ds->getNumMultiSamples();
        
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( new osgQt::GraphicsWindowQt(traits.get()) );
        
        camera->setClearMask( GL_DEPTH_BUFFER_BIT );  // FIXME: could we just render Qt elements later?
        camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
        camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
        camera->setProjectionMatrixAsPerspective(
            30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
        return camera.release();
    }
    
    virtual void paintEvent( QPaintEvent* event )
    { frame(); }

protected:
    MyScene* _scene;
    QTimer _timer;
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc, argv);

    // FIXME: IMPORTANT
    // Still wait for https://codereview.qt-project.org/#change,40644
    // This patch may fix current threading problem, so before that, we only test single threading..
    osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
    while (arguments.read("--SingleThreaded")) threadingModel = osgViewer::ViewerBase::SingleThreaded;
    while (arguments.read("--CullDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
    while (arguments.read("--DrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::DrawThreadPerContext;
    while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext;
    
    QApplication app(argc, argv);
    ViewerWidget* viewWidget = new ViewerWidget(threadingModel);
    viewWidget->setGeometry( 100, 100, 800, 600 );
    viewWidget->show();
    return app.exec();
}
