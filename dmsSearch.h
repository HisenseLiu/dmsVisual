/**********************************************************************************************************************
���ܣ�����������·������Χ���������㷨��
����ʱ�䣺2020.12.26
�����ߣ������š�
˵�������ڶ��ӡ��ڵ�ģ�ͽ���������
***********************************************************************************************************************/

#ifndef CDMSSEARCH_INCLUDED
#define CDMSSEARCH_INCLUDED

class CPscTree;
class CTreeNode;
class CTreeNodeData;

//����������
class CDmsSearch
{
public:
    CDmsSearch();
    ~CDmsSearch();

    void SearchAutoBkEndTree( int iEqId, CPscTree* pDmsTree );            //����ָ���豸�������Զ������ؽ�β����
    void SetSearchFeederId( int iFeederId ) { m_iFeederId = iFeederId; }  //����Ҫ��������������
    void OutTreeModel( CPscTree& rDmsTree );                              //�����ģ��

protected:
    bool DetermineSearchTe( int iEqId, int& iSearchTeIdx );               //�����豸IDȷ����ʼ�����Ķ�������
    bool SearchNoSwEqFromTe( int iSrcTeIdx, int& iType, int& iTypeIdx );  //��ָ�����������ǿ��ء���բ�豸
    bool DetermineFeederIdByLnIdx( int iType, int iTypeIdx );             //ȷ����������

    bool IsSelfFeedBk( int iEqIdx );                                                 //�Ƿ������豸
    bool IsAutoBk( int iEqIdx );                                                     //�Ƿ��Զ�������
    int  GetEqTeFromTe( int iTeIdx, int& iEqIdx, int& iTe1, int& iTe2, int& iTe3 );  //��ָ�����������豸�������豸�ϵ����ж���
    int  GetEqIdxFromTe( int iTeIdx );                                               //��ȡָ�������������豸����

    void FilterTreeNoAutoBkLeaf( CPscTree& rDmsTree );                  //�������еķǿ��ص�բҶ�ӽڵ�
    void DFS_TravelOutTree( CPscTree& rDmsTree, CTreeNode* pCurNode );  //��ȱ��������ģ��
    int  GetEqIdOfTreeNode( CTreeNode* pCurNode );                      //�����ڵ��ȡ�豸ID

    static void DisposeDmsTreeNode( CTreeNodeData& treeNodeData, void* pUserObjData );  //����Ҷ�ӽڵ��������Ļص�����

private:
    void SearchDmsTree( int iEqId, CPscTree* pDmsTree );                                                                        //����ָ������Զ��Դ���ɿ��ء���բ�γɵ���
    bool BFS_GetTeDownStreamTree( int iSrcTeIdx, CPscTree* pDmsTree );                                                          //�������ָ���������ο��ء���բ���ɵ���
    void InStack( int iTeIdx, int iEqIdx, CPscTree* pDmsTree, CTreeNode*& pParentNode, CTreeNode** pParentStack, int iLevel );  //��ָ��������ջ

private:
    int* m_pTeVisitFlagRcd;  //���ӷ��ʼ�¼��
    int* m_pEqVisitFlagRcd;  //�豸���ʼ�¼��
    int* m_pToVisitstack;    //���ӷ��ʶ�ջ
    int* m_pEqSearchLevel;   //�豸���ʲ�α�

    int m_iToVisitNum;  //�����ʶ��Ӷ�ջ�еĸ���

    int m_iFeederId;  //��¼Ҫ����������

    static int m_siFilterLeafNum;  //Ҷ�ӽڵ����ʱ����ĸ���
};

class CDisposeTreeUserData
{
public:
    CDisposeTreeUserData() : m_pDmsTree( NULL ), m_pDmsSearch( NULL ) {}
    CDisposeTreeUserData( CPscTree* pDmsTree, CDmsSearch* pDmsSearch ) : m_pDmsTree( pDmsTree ), m_pDmsSearch( pDmsSearch ) {}
    ~CDisposeTreeUserData() {}

    CPscTree*   m_pDmsTree;    //Ҫ�������ָ��
    CDmsSearch* m_pDmsSearch;  //�ص�������Ҫ�Ķ����û�����
};

#endif
