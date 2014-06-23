
#ifndef DISPLAY_DISPLAY_DATA_OPTIONS_H_
#define DISPLAY_DISPLAY_DATA_OPTIONS_H_

#include <map>

namespace casa {
	namespace viewer {

		class DisplayDataOptions {
		public:

			typedef std::map<string,string> map_type;
			typedef map_type::iterator iterator;
			typedef map_type::const_iterator const_iterator;
			typedef map_type::reverse_iterator reverse_iterator;
			typedef map_type::const_reverse_iterator const_reverse_iterator;

			iterator begin( ) {
				return options.begin();
			}
			iterator end( ) {
				return options.end( );
			}
			const_iterator begin( ) const {
				return options.begin( );
			}
			const_iterator end( ) const {
				return options.end( );
			}
			reverse_iterator rbegin( ) {
				return options.rbegin( );
			}
			reverse_iterator rend( ) {
				return options.rend( );
			}
			const_reverse_iterator rbegin( ) const {
				return options.rbegin( );
			}
			const_reverse_iterator rend( ) const {
				return options.rend( );
			}

			string &operator[]( const string &key ) {
				return options[key];
			}
			const string &operator[]( const string &key ) const {
				return ((DisplayDataOptions*)this)->options[key];
			}

			map_type::size_type size( ) const {
				return options.size( );
			}

			void insert( const string &key, const string &value ) {
				options.insert(std::pair<string,string>(key,value));
			}

			bool empty( ) const {
				return options.empty( );
			}

		private:
			map_type options;
		};
	}
}

#endif
