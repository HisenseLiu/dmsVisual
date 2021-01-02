/**********************************************************************************************************************
功能：用于配网中路径、范围等搜索的算法。
创建时间：2020.12.26
创建者：刘海信。
说明：基于端子、节点模型进行搜索。
***********************************************************************************************************************/

#ifndef CDMSSEARCH_INCLUDED
#define CDMSSEARCH_INCLUDED

class CPscTree;
class CTreeNode;
class CTreeNodeData;

//配网搜索类
class CDmsSearch
{
public:
    CDmsSearch();
    ~CDmsSearch();

    void SearchAutoBkEndTree( int iEqId, CPscTree* pDmsTree );            //搜索指定设备下游由自动化开关结尾的树
    void SetSearchFeederId( int iFeederId ) { m_iFeederId = iFeederId; }  //设置要进行搜索的馈线
    void OutTreeModel( CPscTree& rDmsTree );                              //输出树模型

protected:
    bool DetermineSearchTe( int iEqId, int& iSearchTeIdx );               //根据设备ID确定开始搜索的端子索引
    bool SearchNoSwEqFromTe( int iSrcTeIdx, int& iType, int& iTypeIdx );  //从指定端子搜索非开关、刀闸设备
    bool DetermineFeederIdByLnIdx( int iType, int iTypeIdx );             //确定所属馈线

    bool IsSelfFeedBk( int iEqIdx );                                                 //是否本馈线设备
    bool IsAutoBk( int iEqIdx );                                                     //是否自动化开关
    int  GetEqTeFromTe( int iTeIdx, int& iEqIdx, int& iTe1, int& iTe2, int& iTe3 );  //从指定端子搜索设备索引及设备上的所有端子
    int  GetEqIdxFromTe( int iTeIdx );                                               //获取指定端子所属的设备索引

    void FilterTreeNoAutoBkLeaf( CPscTree& rDmsTree );                  //过滤树中的非开关刀闸叶子节点
    void DFS_TravelOutTree( CPscTree& rDmsTree, CTreeNode* pCurNode );  //深度遍历输出树模型
    int  GetEqIdOfTreeNode( CTreeNode* pCurNode );                      //从树节点获取设备ID

    static void DisposeDmsTreeNode( CTreeNodeData& treeNodeData, void* pUserObjData );  //对树叶子节点遍历处理的回调函数

private:
    void SearchDmsTree( int iEqId, CPscTree* pDmsTree );                                                                        //搜索指定端子远电源侧由开关、刀闸形成的树
    bool BFS_GetTeDownStreamTree( int iSrcTeIdx, CPscTree* pDmsTree );                                                          //广度搜索指定端子下游开关、刀闸构成的树
    void InStack( int iTeIdx, int iEqIdx, CPscTree* pDmsTree, CTreeNode*& pParentNode, CTreeNode** pParentStack, int iLevel );  //将指定端子入栈

private:
    int* m_pTeVisitFlagRcd;  //端子访问记录表
    int* m_pEqVisitFlagRcd;  //设备访问记录表
    int* m_pToVisitstack;    //端子访问堆栈
    int* m_pEqSearchLevel;   //设备访问层次表

    int m_iToVisitNum;  //待访问端子堆栈中的个数

    int m_iFeederId;  //记录要搜索的馈线

    static int m_siFilterLeafNum;  //叶子节点遍历时处理的个数
};

class CDisposeTreeUserData
{
public:
    CDisposeTreeUserData() : m_pDmsTree( NULL ), m_pDmsSearch( NULL ) {}
    CDisposeTreeUserData( CPscTree* pDmsTree, CDmsSearch* pDmsSearch ) : m_pDmsTree( pDmsTree ), m_pDmsSearch( pDmsSearch ) {}
    ~CDisposeTreeUserData() {}

    CPscTree*   m_pDmsTree;    //要处理的树指针
    CDmsSearch* m_pDmsSearch;  //回调函数需要的额外用户数据
};

#endif
