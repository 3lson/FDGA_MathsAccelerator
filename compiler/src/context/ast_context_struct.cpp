#include "../../include/context/ast_context.hpp"

namespace ast {

// Helper function to get type alignment
int get_type_alignment(Type type) {
    auto it = types_size.find(type);
    return (it != types_size.end()) ? it->second : 4; // Default alignment of 4 if type is unknown
}

// Initializes a struct definition, handling alignment and padding
void Context::struct_init(const std::string& identifier,
        const std::unordered_map<std::string, Type>& structMembers) {
    std::unordered_map<std::string, int> structOffsets;
    int currentOffset = 0;
    int maxAlignment = 1;

    for (const auto& [memberName, memberType] : structMembers) {
        int alignment = get_type_alignment(memberType);
        int size = types_size.at(memberType);

        std::cout << "Processing member: " << memberName << " with size: " << size << " and alignment: " << alignment << std::endl;

        // Ensure the current offset is aligned
        if (currentOffset % alignment != 0) {
            int padding = alignment - (currentOffset % alignment);
            std::cout << "Adding padding of " << padding << " bytes to align offset. " << std::endl;
            currentOffset += padding;
        }

        // Assign the aligned offset to the struct member
        structOffsets[memberName] = currentOffset;
        std::cout << "Assigned offset for " << memberName << ":" << currentOffset << std::endl;
        currentOffset += size;
        maxAlignment = std::max(maxAlignment, alignment);
    }

    // Ensure the struct's total size is aligned to its largest member's alignment
    if (currentOffset % maxAlignment != 0) {
        int padding = maxAlignment - (currentOffset % maxAlignment);
        std::cout << "Adding final padding of " << padding << " bytes " << std::endl;
        currentOffset += padding;
    }
    std::cout << "Final struct size after padding: " << currentOffset << std::endl;

    // Register the struct type and its size for use in sizeof
    Type struct_type = get_or_create_struct_type(identifier);
    set_struct_size(struct_type, currentOffset);

    // Store struct details
    structMap[identifier] = structMembers;
    structOffsetMap[identifier] = structOffsets;
    structSizes[identifier] = currentOffset;
}

// Retrieves struct members
std::unordered_map<std::string, Type> Context::get_struct_members(const std::string& identifier) const {
    auto it = structMap.find(identifier);
    if (it != structMap.end()) {
        return it->second;
    }
    throw std::runtime_error("Struct '" + identifier + "' not found.");
}

// Retrieves struct offsets
std::unordered_map<std::string, int> Context::get_struct_offsets(const std::string& identifier) const {
    auto it = structOffsetMap.find(identifier);
    if (it != structOffsetMap.end()) {
        return it->second;
    }
    throw std::runtime_error("Struct '" + identifier + "' offsets not found.");
}

// Checks if a given identifier is a struct
bool Context::is_struct(const std::string& identifier) {
    auto it = structMap.find(identifier);
    if (it == structMap.end()) {
        std::cout << "Identifier not found in structMap: " << identifier << std::endl;
    } else {
        std::cout << "Identifier found in structMap: " << identifier << std::endl;
    }
    return it != structMap.end();
}


// Sets the total size of a struct (if manually adjusted)
void Context::set_struct_size(Type struct_type, int size) {
    std::cout << "Setting struct size: " << static_cast<int>(struct_type) << " -> " << size << std::endl;
    struct_size_map_[struct_type] = size;
}

// Retrieves the size of a struct
int Context::get_struct_size(Type struct_type) const {
    auto it = struct_size_map_.find(struct_type);
    if (it != struct_size_map_.end()) {
        return it->second;
    }
    throw std::runtime_error("Context::get_struct_size - Struct type not found");
}

Type Context::get_or_create_struct_type(const std::string& struct_name){
    if(struct_type_map_.find(struct_name) != struct_type_map_.end()){
        return struct_type_map_[struct_name];
    }
    //Create a new unique type for this struct
    Type new_type = static_cast<Type>(struct_type_map_.size() + static_cast<int>(Type::_CUSTOM_STRUCT));
    struct_type_map_[struct_name] = new_type;
    return new_type;
}

} // namespace ast
