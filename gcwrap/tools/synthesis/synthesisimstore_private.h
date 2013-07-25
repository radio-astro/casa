
friend class synthesisparsync;
friend class synthesisimager;

public:

synthesisimstore(casa::SIImageStore* imstore);
casa::SIImageStore* getImStore();

protected:


private:

//std::auto_ptr<casa::SIImageStore> itsImStore;

casa::SIImageStore* itsImStore;

bool containsimage;
