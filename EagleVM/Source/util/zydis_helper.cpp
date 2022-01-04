#include "util/zydis_helper.h"

std::vector<uint8_t> zydis_helper::encode_request(ZydisEncoderRequest& request)
{
    std::vector<uint8_t> instruction(ZYDIS_MAX_INSTRUCTION_LENGTH);
    ZyanUSize encoded_length = sizeof uint8_t * ZYDIS_MAX_INSTRUCTION_LENGTH;
    ZydisEncoderEncodeInstruction(&request, instruction.data(), &encoded_length);

    instruction.resize(encoded_length);

    return instruction;
}

ZydisEncoderRequest zydis_helper::create_encode_request(const ZydisMnemonic mnemonic)
{
    ZydisEncoderRequest req;
    ZYAN_MEMSET(&req, 0, sizeof(req));
    req.mnemonic = mnemonic;
    req.operand_count = 0;

    return req;
}

void zydis_helper::add_op(ZydisEncoderRequest& req, ZydisImm imm)
{
    auto op_index = req.operand_count;

    req.operands[op_index].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
    req.operands[op_index].imm = imm;
    req.operand_count++;
}

void zydis_helper::add_op(ZydisEncoderRequest& req, ZydisMem mem)
{
    auto op_index = req.operand_count;

    req.operands[op_index].type = ZYDIS_OPERAND_TYPE_MEMORY;
    req.operands[op_index].mem = mem;
    req.operand_count++;
}

void zydis_helper::add_op(ZydisEncoderRequest& req, ZydisPtr ptr)
{
    auto op_index = req.operand_count;
    
    req.operands[op_index].type = ZYDIS_OPERAND_TYPE_POINTER;
    req.operands[op_index].ptr = ptr;
    req.operand_count++;
}

void zydis_helper::add_op(ZydisEncoderRequest& req, ZydisReg reg)
{
    auto op_index = req.operand_count;
    
    req.operands[op_index].type = ZYDIS_OPERAND_TYPE_REGISTER;
    req.operands[op_index].reg = reg;
    req.operand_count++;
}

std::vector<uint8_t> zydis_helper::encode_queue(std::vector<ZydisEncoderRequest>& queue)
{
    std::vector<uint8_t> data;
    for (int i = 0; i < queue.size(); i++)
    {
        std::vector<uint8_t> instruction_data(ZYDIS_MAX_INSTRUCTION_LENGTH);
        ZyanUSize encoded_length = ZYDIS_MAX_INSTRUCTION_LENGTH;

        ZydisEncoderEncodeInstruction(&queue[i], instruction_data.data(), &encoded_length);
        instruction_data.resize(encoded_length);

        data.insert(data.end(), instruction_data.begin(), instruction_data.end());
    }

    return data;
}

void zydis_helper::setup_decoder()
{
    ZydisDecoderInit(&zyids_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
}

std::vector<ZydisDecode> zydis_helper::get_instructions(void* data, size_t size)
{
    std::vector<ZydisDecode> decode_data;

    ZyanUSize offset = 0;
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];
    
    while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(&zyids_decoder, (char*)data + offset, size - offset, &instruction, operands,
        ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY)))
    {
        ZydisDecode decoded = { instruction, 0 };
        std::memcpy(&decoded.operands[0], operands, sizeof operands);

        decode_data.push_back(decoded);
        offset += instruction.length;
    }

    return decode_data;
}