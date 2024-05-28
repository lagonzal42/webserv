
#include <map>

/**
 * @brief	This class is a singleton.  
 * 
 * 			_dict:	pointer to the class in order to not be initialized two times.
 * 			_mime:	map that contains a dictionary that contains the extesion of the file as key
 *          		and the content type as value.
 */

class MimeDict
{
	public:

		class Deleter
		{
			public:
				~Deleter() 
				{
					delete MimeDict::_dict;
				}
		};
	
		~MimeDict(void);
		static MimeDict*					getMimeDict(void);
		static std::map<std::string, std::string>	getMap(void);
	
	private:
		static MimeDict* _dict;
		std::map<std::string, std::string> _mime;
		static Deleter _deleter;
		
		MimeDict(void);
		MimeDict(const MimeDict&) = delete;
	
};