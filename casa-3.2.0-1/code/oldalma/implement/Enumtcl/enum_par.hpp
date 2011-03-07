template<typename val_type>
class EnumPar {
public:
  EnumPar(){};
  EnumPar& operator () ( int id, string str, string desc ){ id_=id; str_=str; desc_=desc; val_=0; return *this; }
  EnumPar& operator () ( int id, string str, string desc, val_type* val ){ id_=id; str_=str; desc_=desc; val_=val; return *this; }
  int      id()  { return id_;   }
  string   str() { return str_; }
  string   desc(){ return desc_; }
  val_type val() { return *val_; }
 
private:
  int       id_;
  string    str_;
  string    desc_;
  val_type* val_;
};
