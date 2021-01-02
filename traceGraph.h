/**********************************************************************************************************************
���ܣ����ݸ��������ṹ, �������չʾ·����html�ļ���
����ʱ��: 2020.12.20
������: ������
***********************************************************************************************************************/

#ifndef CTRACEGRAPH_INCLUDED
#define CTRACEGRAPH_INCLUDED

#include "limits.h"

class CPscTree;  //���ṹǰ������
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

    int m_iDevId;    //�豸ID
    int m_iDevType;  //�豸����
    int m_iDevIdx;   //�豸����
};

//����Graph���ӻ���������
class CGraphConfig
{
};

//����Tree���ӻ���������
class CTreeConfig
{
};

class CTraceGraphHtml
{
public:
    CTraceGraphHtml();     //���캯��
    ~CTraceGraphHtml() {}  //��������

    void SetOutputDir( char* pDir );     //���������Ŀ¼
    void SetFieName( char* pFileName );  //����������ļ���

    bool SetData( CPscTree* m_pTree );  //���ݸ����������ݳ�ʼ��
    bool OutputFile();                  //��ʼ����ļ�
    bool OutputHtml();

private:
    const char* GetDevDesc( int iDevId );

    bool   SetDataJson( CPscTree* pTree );
    bool   SetLinksJson( CPscTree* pTree );
    void   FillupLinksJson( CTreeNode* pCurNode );
    void   FillupOneLinkJson( CTreeNodeData& rParentNodeData, CTreeNodeData& rChildNodeData );
    void   GetJsonInfo( char*& pJsonInfo );  //��ȡjson����
    cJSON* GetDataJson();
    cJSON* GetLinksJson();

private:
    char m_szDir[PATH_MAX];
    char m_szFileName[NAME_MAX];

    cJSON* m_pDataJson;   //�豸��Ϣjson����
    cJSON* m_pLinksJson;  //�豸֮�����ӹ�ϵjson����
};

#endif  //CTRACEGRAPH