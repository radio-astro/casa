// Private part to calibrater_cmpt.h

    casa::MeasurementSet *itsMS;
    casa::Calibrater *itsCalibrater;
    casa::LogIO *itsLog;

    // Set default parameter values
    void defaults();

    // Private method to look up DATA_DESC_ID's for a given SPW_ID
    void ddid(std::vector<int>& dd, const int swid);

    // Private function to generate uv-range TAQL selection strings
    void uvtaql(std::string& uvsel, bool& noselect,
                const std::vector<double>& uvrange);
    // Private function to pre-process input selection strings
    void validstring(std::string& outputstring, const std::string inputstring);
    // Getid - private function to obtain the field_id for a given field name
    bool getfldidlist(std::vector<int>& fieldids,
		      std::vector<std::string>& fieldnames);
    bool fileExists(const std::string& pathName, bool follow);

    // Sink used to store history
    casa::LogSink logSink_p;

