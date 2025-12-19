#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
using namespace std;

// Base file class
class FileEntity {
protected:
    string name, path;
public:
    FileEntity(string n, string p) : name(n), path(p) {}
    virtual void displayInfo() = 0;
    virtual ~FileEntity() {}
    string getName() { return name; }
    string getPath() { return path; }
    void setName(string n) { name = n; }
};

// Text file
class TextFile : public FileEntity {
    string content;
public:
    TextFile(string n, string p, string c = "") : FileEntity(n, p), content(c) {}
    void displayInfo() override {
        cout << "File: " << name << "\nPath: " << path << "\nContent: " << content << "\n";
    }
    string getContent() { return content; }
};

// Directory class
class Directory {
    string dirName;
    Directory* parent;
    vector<FileEntity*> files;
    vector<Directory*> subDirs;
public:
    Directory(string n, Directory* p = nullptr) : dirName(n), parent(p) {}
    string getName() { return dirName; }
    string getPath() { return parent ? parent->getPath() + "/" + dirName : "/" + dirName; }
    Directory* getParent() { return parent; }
    void addFile(FileEntity* f) { files.push_back(f); }
    void addDir(Directory* d) { subDirs.push_back(d); }
    vector<FileEntity*>& getFiles() { return files; }
    vector<Directory*>& getSubDirs() { return subDirs; }

    FileEntity* findFile(string n) {
        for (auto f : files) if (f->getName() == n) return f;
        return nullptr;
    }

    Directory* findSubDir(string n) {
        for (auto d : subDirs) if (d->getName() == n) return d;
        return nullptr;
    }

    void renameFile(string oldN, string newN) {
        FileEntity* f = findFile(oldN);
        if (f) { f->setName(newN); cout << "Renamed.\n"; }
        else cout << "File not found.\n";
    }

    void deleteFile(string n) {
        auto it = remove_if(files.begin(), files.end(), [&](FileEntity* f){ return f->getName() == n; });
        if (it != files.end()) { for (auto f = it; f != files.end(); ++f) delete *f; files.erase(it, files.end()); cout << "Deleted.\n"; }
        else cout << "File not found.\n";
    }

    void deleteDir(string n) {
        auto it = remove_if(subDirs.begin(), subDirs.end(), [&](Directory* d){ return d->getName() == n; });
        if (it != subDirs.end()) { for (auto d = it; d != subDirs.end(); ++d) delete *d; subDirs.erase(it, subDirs.end()); cout << "Deleted.\n"; }
        else cout << "Directory not found.\n";
    }

    void displayContents() {
        cout << "Directory: " << dirName << "\nFiles:\n";
        for (auto f : files) { f->displayInfo(); cout << "---\n"; }
        cout << "Subdirectories:\n";
        for (auto d : subDirs) cout << d->getName() << endl;
    }

    ~Directory() { for (auto f : files) delete f; for (auto d : subDirs) delete d; }
};

Directory* currentDir = nullptr;
set<string> favoriteFiles;

// Menu functions
void pwd() { cout << "Current Directory: " << currentDir->getPath() << endl; }

void createFile() {
    string name, content;
    cout << "File name: "; cin >> name;
    cin.ignore();
    cout << "Content: "; getline(cin, content);
    currentDir->addFile(new TextFile(name, currentDir->getPath(), content));
    cout << "File created.\n";
}

void createDirectory() {
    string name;
    cout << "Directory name: "; cin >> name;
    currentDir->addDir(new Directory(name, currentDir));
    cout << "Directory created.\n";
}

void renameFile() {
    string oldN, newN;
    cout << "Old name: "; cin >> oldN;
    cout << "New name: "; cin >> newN;
    currentDir->renameFile(oldN, newN);
}

void displayFiles() { currentDir->displayContents(); }

void shiftToDirectory() {
    string name; cout << "Directory: "; cin >> name;
    Directory* d = currentDir->findSubDir(name);
    if (d) { currentDir = d; pwd(); }
    else { char c; cout << "Not found. Create? (y/n): "; cin >> c; if (c=='y'||c=='Y'){ currentDir->addDir(new Directory(name,currentDir)); currentDir=currentDir->findSubDir(name); pwd(); } }
}

void deleteFile() {
    string name; cout << "File: "; cin >> name; char c; cout << "Delete? (y/n): "; cin >> c;
    if (c=='y'||c=='Y') { currentDir->deleteFile(name); favoriteFiles.erase(currentDir->getPath()+"/"+name); }
}

void deleteDirectory() {
    string name; cout << "Directory: "; cin >> name; char c; cout << "Delete? (y/n): "; cin >> c;
    if (c=='y'||c=='Y') currentDir->deleteDir(name);
}

void goBackToParent() {
    if(currentDir->getParent()) currentDir=currentDir->getParent(), pwd();
    else cout << "Already at root.\n";
}

void moveToNewDirectory() {
    string name; cout << "New dir: "; cin >> name;
    currentDir->addDir(new Directory(name,currentDir)); currentDir=currentDir->findSubDir(name); pwd();
}

void copyFileToDirectory() {
    string fName, dName;
    cout << "File: "; cin >> fName;
    cout << "Target Dir: "; cin >> dName;
    FileEntity* f = currentDir->findFile(fName);
    Directory* d = currentDir->findSubDir(dName);
    if(!f) { cout << "File not found.\n"; return; }
    if(!d) { cout << "Directory not found.\n"; return; }
    TextFile* tf = dynamic_cast<TextFile*>(f);
    if(tf) d->addFile(new TextFile(fName,d->getPath(),tf->getContent())), cout << "Copied.\n";
    else cout << "Cannot copy.\n";
}

void searchFileRecursive(Directory* dir, string n, vector<string>& res, string path){
    for(auto f:dir->getFiles()) if(f->getName().find(n)!=string::npos) res.push_back(path+"/"+f->getName());
    for(auto d:dir->getSubDirs()) searchFileRecursive(d,n,res,path+"/"+d->getName());
}

void searchFile() {
    string name; cout << "File: "; cin >> name;
    vector<string> res; searchFileRecursive(currentDir,name,res,currentDir->getPath());
    if(res.empty()) cout << "No files found.\n"; else for(auto &s:res) cout << s << endl;
}

void markAsFavorite() {
    string name; cout << "File: "; cin >> name;
    if(currentDir->findFile(name)) favoriteFiles.insert(currentDir->getPath()+"/"+name), cout << "Marked favorite.\n";
    else cout << "File not found.\n";
}

void displayFavoriteFiles() {
    if(favoriteFiles.empty()) cout << "No favorites.\n"; else for(auto &f:favoriteFiles) cout << f << endl;
}

// Main menu
int main(){
    Directory* root = new Directory("root"); currentDir=root;
    int choice;
    do{
        cout << "\n=== Current Directory: " << currentDir->getPath() << " ===\n";
        cout << "1.Create File  2.Create Dir  3.Rename File  4.Display Files\n";
        cout << "5.Navigate Dir 6.Delete File 7.Delete Dir 8.Go Back\n";
        cout << "9.New Dir & Move 10.Copy File 11.Search File 12.Mark Favorite 13.Display Favorites 0.Exit\nChoice: ";
        cin >> choice;
        switch(choice){
            case 1:createFile(); break; case 2:createDirectory(); break; case 3:renameFile(); break; case 4:displayFiles(); break;
            case 5:shiftToDirectory(); break; case 6:deleteFile(); break; case 7:deleteDirectory(); break; case 8:goBackToParent(); break;
            case 9:moveToNewDirectory(); break; case 10:copyFileToDirectory(); break; case 11:searchFile(); break;
            case 12:markAsFavorite(); break; case 13:displayFavoriteFiles(); break; case 0: cout << "Exiting...\n"; break;
            default: cout << "Invalid choice.\n";
        }
        if(choice!=0){ cout << "Press Enter to continue..."; cin.ignore(); cin.get(); }
    }while(choice!=0);
    delete root; currentDir=nullptr;
}
