2025-08-10
- [X] Center image when smaller than 
- [X] Compat: iterate/iterate_
- [X] Return RGB(r, g, b)
- [X] Enable/disable buttons
- [X] Params table
- [X] Thumbnail
- [X] Set parameter when switching to dspace
- [X] Tabulate color map
- [X] Maintain aspect ratio when selecting
- [X] Zoom in and out (ctrl-mousewheel), reset zoom
- [X] Obtain colormaps
- [X] Colormap changing
- [X] Colormap preview 
- [X] Sandbox
- [X] Iterate current point
- [X] Draw figure, iterate (1/2/3/...)
- [X] Built-in functions (loaded by default)
- [X] Functions tree
- [X] Compilation
- [X] Artifacts bug: QImage not thread-safe, use map()
- [X] Better tile scheduling / parallelization
- [X] Settings: mode, last function, window
- [X] Check all compiled functions against old version
- [X] Code editor
- [X] Show compiliation errors, click on error line
- [X] App icon https://doc.qt.io/qt-6/appicon.html
- [X] Export image
- [X] Printing
- [X] Debugging!
- [X] Wider parameter section
- [X] Virgin installation experience (no directory), packmaps
- [X] Deploy.sh
- [X] Install maps and include files
- [X] Settings; files directory: platform-dependent - choosable
- [X] Notebook integration
- [X] Function tree manipulation
- [X] Dark mode
- [X] Collapsible treeview with button to open it
- [X] Help page and cheat sheet
- [X] Annotations: function vs temporary (orbit, Clear from index)
- [X] Randomize colors (R -> drawing colors)
- [X] Need two save/compile to fix an error (runs wrong version?)
- [X] Put when to compile in code, compile unconditionally
- [X] Github
- [X] Show code for samples (read only)
- [X] Link for manual -> github version
- [X] Open Jupyter in browser (not webview)
-- 10/09/2025 Done ------------------------------------------------------------
- [X] MacOS packaging/signing/DMG -- approx Sep 10
-- 20/09/2025 Bugs ------------------------------------------------------------
- [X] RGB: return rgb(int,int,int)
- [X] Artifacts with multithreading z^4+a/z^4, z^2*(z-a), a(exp(z)-1)
- [X] a(exp(z)-1): enable debug-> multithreading artifacts
- [X] Error window doesn't go away
- [X] Stop button not working
- [X] Stopping when single threaded
- [X] Better indication of when running
- [X] Pick place in dspace, go back: coordinates bad
- [X] Esc to reset zoom/exit whatever (like clicking)
-------------------------------------------------------------------------------
- [X] Update manual
- [X] Windows version
- [X] Linux version

- [X] Colormap bug (casting etc)
- [X] Default size 1000, max higher, weirdness editing size by hand
- [X] Add built-in functions
- [X] Thumbnail: limit size
- [X] Functions cannot be moved up or down in list
- [ ] Save PDF: tiny picture

- [X] Custom colormap in code: rgb
- [ ] Get better colormaps; remove some maps
- [ ] Change colormap while computing: disable

- [ ] Bookmark places

---Ideas-----------------------------------------------------------------------
- [ ] Qt showcase (app store?)
- [ ] Compute servers (other copies)
- [ ] Automatic copy function - new style functions 

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
# Refine Algorithm
Objectives:
    - Parallelize 
    - Preview every 100-250 ms
    - Keep cores busy even w/ different load per region
    - Cache-locality

Given:
    - pixels per second (per core)
    - picture size

Subdivide picture into small enough regions (16x16 - 32x32 pixels)
Do tiles in phases:
    - Phase 1 - single pix colors whole tile
    - Phase 2 - subdivide in 4 (3 more pixels)
    - Rest - calculate all.

Cache sizes: there are per-core caches, about 48k-128k.
An image is much larger, 3-8 MB. Memory organization is line-by line.
-> Forget about cache locality for now.

What if the tiles had their own memory that is mapped (it needs to be mapped
anyway) - this would give us better cache locality.

Results:
1000 x 1000 Mandi
single threaded: 765 (1307 pix/ms)
stripes 12 cores: 200 (5000 pix/ms)
Oldtiles 12 cores: 110 (9090 pix/ms)
Newtiles: 100 (10000 pix/ms)
    128x128     130
    64x64       100
    50x50       100 <-- keep at 50, multiple of sizes
    32x32       110
    16x16       170
-------------------------------------------------------------------------------
play-outline
close-outline
arrow-back-outline
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
# Icons
https://ionic.io/ionicons (Alternatives: search for "popular modern svg icon pack")
Icon files are of the form:
```svg
  <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-activity"><polyline points="22 12 18 12 15 21 9 3 6 12 2 12"></polyline></svg>
```
Before saving, you should replace "currentColor" with #E0E0E0 for dark mode and #424242 for light mode.
You can also change stroke-width and size (replacing all occurrences of 24).
It icons are 32x32, which is slightly too large. Standard is 24.

### TreeView usage
```c++
#include "HierarchicalTreeView.h"
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

// Example data structure
struct MyData {
    int id;
    QString description;
    
    MyData(int i, const QString& desc) : id(i), description(desc) {}
};

class ExampleMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ExampleMainWindow(QWidget* parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupData();
        connectSignals();
    }

private slots:
    void onItemDoubleClicked(HierarchicalItem* item)
    {
        QString info = QString("Double-clicked: %1").arg(item->displayName());
        if (item->data()) {
            MyData* data = static_cast<MyData*>(item->data());
            info += QString(" (ID: %1, Desc: %2)").arg(data->id).arg(data->description);
        }
        m_statusLabel->setText(info);
    }
    
    void onItemSelectionChanged(HierarchicalItem* current, HierarchicalItem* previous)
    {
        Q_UNUSED(previous)
        if (current) {
            QString info = QString("Selected: %1 [%2] - Path: %3")
                          .arg(current->displayName())
                          .arg(current->isFolder() ? "Folder" : "Item")
                          .arg(current->path());
            m_statusLabel->setText(info);
        }
    }
    
    void onItemMoved(HierarchicalItem* item, HierarchicalItem* oldParent, HierarchicalItem* newParent)
    {
        QString info = QString("Moved '%1' from '%2' to '%3'")
                      .arg(item->displayName())
                      .arg(oldParent->displayName())
                      .arg(newParent->displayName());
        m_statusLabel->setText(info);
    }

private:
    void setupUI()
    {
        auto* centralWidget = new QWidget;
        setCentralWidget(centralWidget);
        
        auto* layout = new QVBoxLayout(centralWidget);
        
        // Create the tree view and model
        m_model = new HierarchicalModel(this);
        m_treeView = new HierarchicalTreeView(this);
        m_treeView->setHierarchicalModel(m_model);
        
        // Add some buttons for demonstration
        auto* buttonLayout = new QHBoxLayout;
        auto* addFolderBtn = new QPushButton("Add Folder", this);
        auto* addItemBtn = new QPushButton("Add Item", this);
        auto* printTreeBtn = new QPushButton("Print Tree", this);
        auto* findItemBtn = new QPushButton("Find Item", this);
        
        buttonLayout->addWidget(addFolderBtn);
        buttonLayout->addWidget(addItemBtn);
        buttonLayout->addWidget(printTreeBtn);
        buttonLayout->addWidget(findItemBtn);
        buttonLayout->addStretch();
        
        // Status label
        m_statusLabel = new QLabel("Ready", this);
        
        layout->addLayout(buttonLayout);
        layout->addWidget(m_treeView);
        layout->addWidget(m_statusLabel);
        
        // Connect buttons
        connect(addFolderBtn, &QPushButton::clicked, m_treeView, &HierarchicalTreeView::addFolder);
        connect(addItemBtn, &QPushButton::clicked, m_treeView, &HierarchicalTreeView::addItem);
        connect(printTreeBtn, &QPushButton::clicked, this, &ExampleMainWindow::printTree);
        connect(findItemBtn, &QPushButton::clicked, this, &ExampleMainWindow::findItemExample);
        
        setWindowTitle("Hierarchical Tree View Example");
        resize(600, 400);
    }
    
    void setupData()
    {
        // Create a sample hierarchy
        HierarchicalItem* root = m_model->rootItem();
        
        // Create some folders
        auto* documentsFolder = new HierarchicalItem("Documents", HierarchicalItem::Folder);
        auto* projectsFolder = new HierarchicalItem("Projects", HierarchicalItem::Folder);
        auto* imagesFolder = new HierarchicalItem("Images", HierarchicalItem::Folder);
        
        root->appendChild(documentsFolder);
        root->appendChild(projectsFolder);
        root->appendChild(imagesFolder);
        
        // Add items to Documents
        auto* doc1 = new HierarchicalItem("Report.docx", HierarchicalItem::Item, 
                                         new MyData(1, "Quarterly report"));
        auto* doc2 = new HierarchicalItem("Notes.txt", HierarchicalItem::Item, 
                                         new MyData(2, "Meeting notes"));
        documentsFolder->appendChild(doc1);
        documentsFolder->appendChild(doc2);
        
        // Add a subfolder to Projects
        auto* qtProjectFolder = new HierarchicalItem("Qt Application", HierarchicalItem::Folder);
        projectsFolder->appendChild(qtProjectFolder);
        
        auto* mainCpp = new HierarchicalItem("main.cpp", HierarchicalItem::Item, 
                                           new MyData(3, "Main application file"));
        auto* headerFile = new HierarchicalItem("app.h", HierarchicalItem::Item, 
                                              new MyData(4, "Header file"));
        qtProjectFolder->appendChild(mainCpp);
        qtProjectFolder->appendChild(headerFile);
        
        // Add items to Images
        auto* logo = new HierarchicalItem("logo.png", HierarchicalItem::Item, 
                                        new MyData(5, "Company logo"));
        auto* banner = new HierarchicalItem("banner.jpg", HierarchicalItem::Item, 
                                          new MyData(6, "Website banner"));
        imagesFolder->appendChild(logo);
        imagesFolder->appendChild(banner);
        
        // Expand the tree to show the structure
        m_treeView->expandAll();
    }
    
    void connectSignals()
    {
        connect(m_treeView, &HierarchicalTreeView::itemDoubleClicked,
                this, &ExampleMainWindow::onItemDoubleClicked);
        connect(m_treeView, &HierarchicalTreeView::itemSelectionChanged,
                this, &ExampleMainWindow::onItemSelectionChanged);
        connect(m_model, &HierarchicalModel::itemMoved,
                this, &ExampleMainWindow::onItemMoved);
        connect(m_model, &HierarchicalModel::itemRenamed,
                this, &ExampleMainWindow::onItemRenamed);
    }
    
    void printTree()
    {
        QString output = "Tree Structure:\n";
        printItem(m_model->rootItem(), output, 0);
        qDebug() << output;
        m_statusLabel->setText("Tree structure printed to console");
    }
    
    void printItem(HierarchicalItem* item, QString& output, int depth)
    {
        QString indent = QString("  ").repeated(depth);
        QString type = item->isFolder() ? "[Folder]" : "[Item]";
        output += QString("%1%2 %3\n").arg(indent, item->displayName(), type);
        
        if (item->data() && item->isItem()) {
            MyData* data = static_cast<MyData*>(item->data());
            output += QString("%1  -> ID: %2, Desc: %3\n")
                     .arg(indent).arg(data->id).arg(data->description);
        }
        
        for (HierarchicalItem* child : item->children()) {
            printItem(child, output, depth + 1);
        }
    }
    
    void findItemExample()
    {
        // Example of finding items
        HierarchicalItem* root = m_model->rootItem();
        
        // Find by name
        HierarchicalItem* found = root->findChild("main.cpp", true);
        if (found) {
            QModelIndex index = m_model->indexFromItem(found);
            m_treeView->setCurrentIndex(index);
            m_statusLabel->setText(QString("Found item: %1 at path: %2")
                                  .arg(found->displayName()).arg(found->path()));
        } else {
            m_statusLabel->setText("Item 'main.cpp' not found");
        }
    }
    
    void onItemRenamed(HierarchicalItem* item, const QString& oldName, const QString& newName)
    {
        QString info = QString("Renamed '%1' to '%2'").arg(oldName, newName);
        m_statusLabel->setText(info);
    }

private:
    HierarchicalModel* m_model;
    HierarchicalTreeView* m_treeView;
    QLabel* m_statusLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ExampleMainWindow window;
    window.show();
    
    return app.exec();
}

// Example of advanced usage - custom traversal and manipulation
class AdvancedExample
{
public:
    static void demonstrateAdvancedFeatures(HierarchicalModel* model)
    {
        HierarchicalItem* root = model->rootItem();
        
        // Find all items with specific criteria
        auto cppFiles = root->findAll([](HierarchicalItem* item) {
            return item->displayName().endsWith(".cpp");
        }, true);
        
        qDebug() << "Found" << cppFiles.size() << "C++ files";
        
        // Path-based access
        HierarchicalItem* item = root->itemAtPath("Projects/Qt Application/main.cpp");
        if (item) {
            qDebug() << "Found item at path:" << item->displayName();
        }
        
        // Programmatic manipulation
        if (!cppFiles.isEmpty()) {
            HierarchicalItem* cppFile = cppFiles.first();
            HierarchicalItem* newFolder = new HierarchicalItem("Source Files", HierarchicalItem::Folder);
            
            // Add new folder to root
            root->appendChild(newFolder);
            
            // Move C++ file to new folder
            cppFile->moveTo(newFolder);
            
            qDebug() << "Moved" << cppFile->displayName() << "to" << newFolder->displayName();
        }
        
        // Custom data access
        auto itemsWithData = root->findAll([](HierarchicalItem* item) {
            return item->data() != nullptr;
        }, true);
        
        for (HierarchicalItem* item : itemsWithData) {
            if (MyData* data = static_cast<MyData*>(item->data())) {
                qDebug() << "Item:" << item->displayName() 
                         << "ID:" << data->id 
                         << "Description:" << data->description;
            }
        }
    }
};

```
compile_windows.bat test "C:/Users/christ/AppData/Roaming/Mannes Technology/It/" "C:/Users/christ/Code/it/build/Desktop_Qt_6_10_0_MSVC2022_ARM64-Debug/It.exe"
