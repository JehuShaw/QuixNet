#include "TemplateBase.h"
#include "CsvParser.h"
#include "NodeDefines.h"
#include "Log.h"

using namespace util;

bool CTemplateBase::Init(const std::string& strPath,
    const std::string& strFileName)
{
    if(!m_bInit) {
        m_bInit = true;
        std::string strFullPath;
        if(!strPath.empty()) {
            strFullPath = strPath;
            int nLastIdx = strFullPath.size() - 1;
            if(strFullPath[nLastIdx] != '/'
                                && !(strFullPath[nLastIdx] == '\\' &&
                (nLastIdx > 0 && strFullPath[nLastIdx - 1] == '\\')))
            {
                strFullPath += '/';
            }
            strFullPath += strFileName;
        }
        csv_parser file_parser;
        //file_parser.set_skip_lines(1);
        /* Specify the file to parse */
        if(!file_parser.init(strFullPath.c_str())) {
            m_bInit = false;
            return false;
        }
        /* Here we tell the parser how to parse the file */
        file_parser.set_enclosed_char(csv_enclosure_char, ENCLOSURE_OPTIONAL);
        file_parser.set_field_term_char(csv_field_terminator);
        file_parser.set_line_term_char(csv_line_terminator);
        // first field line
        if(file_parser.has_more_rows()) {
            csv_row field_name(file_parser.get_row());
            /* Check to see if there are more records, then grab each row one at a time */
            csv_columns_t csvColumns;
            while(file_parser.has_more_rows())
            {
                csv_row row(file_parser.get_row());
                assert(row.size() == field_name.size());
                for(int i = 0; i < (int)row.size(); ++i)
                {
                    std::pair<csv_columns_t::iterator, bool> pairIB(
                    csvColumns.insert(csv_columns_t::value_type(
                    field_name[i], row[i])));
                    if(!pairIB.second) {
                        if(!row[i].empty()) {
                            pairIB.first->second = row[i];
                        }
                    }
                }
                OnRowData(csvColumns);
            }
        }
        return true;
    }
    return false;
}

