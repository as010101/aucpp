#include <iostream>
#include <set>
#include <string>

using namespace std;

void printUsage()
{
	cerr << "Usage: alselect [group1] [group2] [group3]..." << endl
		 << endl;
}

set<string> getWantedGroupNames(int argc, const char* argv[])
{
	set<string> groups;
	for (int i = 1; i < argc; ++i)
		{
			groups.insert(string(argv[i]));
		}

	return groups;
}

typedef enum 
	{
		LT_COLHEADER,
		LT_WHITESPACE,
		LT_MSG_FIRSTLINE,
		LT_MSG_OTHERLINE
	} LINETYPE;


LINETYPE getLineType(const string & aLine)
{
	if (aLine.length() == 0)
		{
			return LT_WHITESPACE;
		}
	if (aLine[0] == '2')
		{
			return LT_MSG_FIRSTLINE;
		}
	else if ((aLine[0] == '=') || (aLine[0] == 'Y'))
		{
			return LT_COLHEADER;
		}
	else
		{
			return LT_MSG_OTHERLINE;
		}
}

// This is only defined when passed the first line of a message.
string getGroupName(const string & aLine)
{
	// Group is between 3rd and 4th '|' chars...
	string::size_type pos3;
	pos3 = aLine.find_first_of('|');
	pos3 = aLine.find_first_of('|', pos3 + 1);
	pos3 = aLine.find_first_of('|', pos3 + 1);

	string::size_type pos4 = aLine.find_first_of('|', pos3 + 1);

	string groupCol = aLine.substr(pos3, pos4 - pos3 + 1);

	string::size_type posGroupNameStart = 2;
	string::size_type posGroupNameEnd   = groupCol.find_first_of(' ', 2) - 1;

	return groupCol.substr(posGroupNameStart, 
						   posGroupNameEnd - posGroupNameStart + 1);
}

int main(int argc, const char* argv[])
{
	set<string> groups = getWantedGroupNames(argc, argv);

	if (groups.size() == 0)
		{
			printUsage();
			return 1;
		}

	// This being the empty string means we're not currently parsing an 
	// interesting group...
	string currentGroup;
	string aLine;

	while (! cin.eof())
		{
			getline(cin, aLine, '\n');
			LINETYPE lt = getLineType(aLine);
			
			if (lt == LT_COLHEADER)
				{
					cout << aLine << endl;
				}
			else if (lt == LT_MSG_FIRSTLINE)
				{
					currentGroup = getGroupName(aLine);

					if (groups.find(currentGroup) != groups.end())
						{
							cout << aLine << endl;
						}
					else
						{
							currentGroup = "";
						}

				}
			else if (lt == LT_MSG_OTHERLINE)
				{
					if (currentGroup != "")
						{
							cout << aLine << endl;
						}
				}
			else
				{
					assert(lt == LT_WHITESPACE);
					if (currentGroup != "")
						{
							cout << endl;
							currentGroup = "";
						}
				}
		}
}
