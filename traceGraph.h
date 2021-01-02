/**********************************************************************************************************************
功能：根据给定的树结构, 输出用于展示路径的html文件。
创建时间: 2020.12.20
创建者: 刘海信
***********************************************************************************************************************/

#ifndef CTRACEGRAPH_INCLUDED
#define CTRACEGRAPH_INCLUDED

#include "limits.h"

class CPscTree;  //树结构前置声明
struct cJSON;

#define OUT_BK  1
#define FEED_BK 2

class CTreeDev
{
public:
    CTreeDev() : m_iDevId( -1 ), m_iDevType( -1 ), m_iDevIdx( -1 ) {}
    CTreeDev( int iDevId, int iDevType, int iDevIdx ) : m_iDevId( iDevId ), m_iDevType( iDevType ), m_iDevIdx( iDevIdx ) {}
    CTreeDev( const CTreeDev& c_st ) : m_iDevId( c_st.m_iDevId ), m_iDevType( c_st.m_iDevType ), m_iDevIdx( c_st.m_iDevIdx ) {}

    CTreeDev& operator=( const CTreeDev& c_st )
    {
        if( this == &c_st )
            return *this;

        m_iDevId   = c_st.m_iDevId;
        m_iDevType = c_st.m_iDevType;
        m_iDevIdx  = c_st.m_iDevType;
    }

    int m_iDevId;    //设备ID
    int m_iDevType;  //设备类型
    int m_iDevIdx;   //设备索引
};

//用于Graph可视化的配置类
class CGraphConfig
{
};

//用于Tree可视化的配置类
class CTreeConfig
{
};

class CTraceGraphHtml
{
public:
    CTraceGraphHtml();     //构造函数
    ~CTraceGraphHtml() {}  //析构函数

    void SetOutputDir( char* pDir );     //设置输出的目录
    void SetFieName( char* pFileName );  //设置输出的文件名

    bool SetData( CPscTree* m_pTree );  //根据给定的树数据初始化
    bool OutputFile();                  //开始输出文件
    bool OutputHtml();

private:
    const char* GetDevDesc( int iDevId );

    bool   SetDataJson( CPscTree* pTree );
    bool   SetLinksJson( CPscTree* pTree );
    void   FillupLinksJson( CTreeNode* pCurNode );
    void   FillupOneLinkJson( CTreeNodeData& rParentNodeData, CTreeNodeData& rChildNodeData );
    void   GetJsonInfo( char*& pJsonInfo );  //获取json数据
    cJSON* GetDataJson();
    cJSON* GetLinksJson();

private:
    char m_szDir[PATH_MAX];
    char m_szFileName[NAME_MAX];

    cJSON* m_pDataJson;   //设备信息json数组
    cJSON* m_pLinksJson;  //设备之间连接关系json数组
};

#endif  //CTRACEGRAPH