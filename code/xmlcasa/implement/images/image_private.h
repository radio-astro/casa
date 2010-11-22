/* Private parts of image component */
public:

// Allow other components that return image tool to open an image
bool open(const casa::ImageInterface<casa::Float>* inImage);

private:


casa::LogIO *itsLog;
casa::ImageAnalysis *itsImage;


// Private ImageInterface constructor to make components on the fly
image(const casa::ImageInterface<casa::Float>* inImage);

image(casa::ImageInterface<casa::Float>* inImage, const bool cloneInputPointer);


// Having private version of IS and IH means that they will
// only recreate storage images if they have to

// Prints an error message if the image DO is detached and returns True.
bool detached() const;

casac::record* recordFromQuantity(casa::Quantity q);
casac::record* recordFromQuantity(const casa::Quantum<casa::Vector<casa::Double> >& q);
casa::Quantity casaQuantityFromVar(const ::casac::variant& theVar);

