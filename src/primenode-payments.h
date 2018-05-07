

// Copyright (c) 2014-2015 The Dash developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PRIMENODE_PAYMENTS_H
#define PRIMENODE_PAYMENTS_H

#include "sync.h"
#include "net.h"
#include "key.h"
#include "util.h"
#include "base58.h"
#include "main.h"
#include "primenode.h"

using namespace std;

class CPrimenodePayments;
class CPrimenodePaymentWinner;

extern CPrimenodePayments primenodePayments;
extern map<uint256, CPrimenodePaymentWinner> mapSeenPrimenodeVotes;

void ProcessMessagePrimenodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);


// for storing the winning payments
class CPrimenodePaymentWinner
{
public:
    int nBlockHeight;
    CTxIn vin;
    CScript payee;
    std::vector<unsigned char> vchSig;
    uint64_t score;

    CPrimenodePaymentWinner() {
        nBlockHeight = 0;
        score = 0;
        vin = CTxIn();
        payee = CScript();
    }

    uint256 GetHash(){
        uint256 n2 = Hash(BEGIN(nBlockHeight), END(nBlockHeight));
        uint256 n3 = vin.prevout.hash > n2 ? (vin.prevout.hash - n2) : (n2 - vin.prevout.hash);

        return n3;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nBlockHeight);
        READWRITE(payee);
        READWRITE(vin);
        READWRITE(score);
        READWRITE(vchSig);
    )
};

//
// Primenode Payments Class
// Keeps track of who should get paid for which blocks
//

class CPrimenodePayments
{
private:
    std::vector<CPrimenodePaymentWinner> vWinning;
    int nSyncedFromPeer;
    std::string strMasterPrivKey;
    std::string strMainPubKey;
    bool enabled;
    int nLastBlockHeight;

public:

    CPrimenodePayments() {
        strMainPubKey = "0430a870a5a1e8a85b816c64cb86d6aa4955134efe72c458246fb84cfd5221c111ee84dfc0dbf160d339415044259519eae2840ab3ffe86368f3f9a93ec22e3f4d";
        enabled = false;
    }

    bool SetPrivKey(std::string strPrivKey);
    bool CheckSignature(CPrimenodePaymentWinner& winner);
    bool Sign(CPrimenodePaymentWinner& winner);

    // Deterministically calculate a given "score" for a primenode depending on how close it's hash is
    // to the blockHeight. The further away they are the better, the furthest will win the election
    // and get paid this block
    //

    uint64_t CalculateScore(uint256 blockHash, CTxIn& vin);
    bool GetWinningPrimenode(int nBlockHeight, CTxIn& vinOut);
    bool AddWinningPrimenode(CPrimenodePaymentWinner& winner);
    bool ProcessBlock(int nBlockHeight);
    void Relay(CPrimenodePaymentWinner& winner);
    void Sync(CNode* node);
    void CleanPaymentList();
    int LastPayment(CPrimenode& mn);
    int GetMinPrimenodePaymentsProto();

    bool GetBlockPayee(int nBlockHeight, CScript& payee, CTxIn& vin);
};


#endif