#pragma once

struct TMeshSubGroup {
  uint32_t first_idx;
  uint32_t num_indices;
  uint32_t material_index;
  uint32_t user_material_id;
};

typedef std::vector<TMeshSubGroup> VMeshSubGroups;
