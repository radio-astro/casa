
friend class synthesisparsync;
friend class synthesisimager;

protected:

casa::SIImageStore* getImStore();

private:

//std::auto_ptr<casa::SIImageStore> itsImStore;

casa::SIImageStore* itsImStore;

synthesisimstore(casa::SIImageStore* imstore);

bool containsimage;
