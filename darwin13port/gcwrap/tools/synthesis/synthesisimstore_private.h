
friend class synthesisparsync;
friend class synthesisimager;


protected:

synthesisimstore(casa::SIImageStore* imstore);
casa::SIImageStore* getImStore();

private:

//std::auto_ptr<casa::SIImageStore> itsImStore;

casa::SIImageStore* itsImStore;

bool containsimage;
