#include "stdafx.h"
#include "buttons.h"

toolbar_extension::button::custom_image & toolbar_extension::button::custom_image::operator = (const toolbar_extension::button::custom_image & p_source)
{
	m_path = p_source.m_path;
	m_mask_type = p_source.m_mask_type;
	//				if (m_mask_type == uie::MASK_BITMAP)
	m_mask_path = p_source.m_mask_path;
	//				else if (m_mask_type == uie::MASK_COLOUR)
	m_mask_colour = p_source.m_mask_colour;
	return *this;
}

void toolbar_extension::button::custom_image::get_path(pfc::string8 & p_out) const
{
	p_out.reset();

	bool b_absolute = pfc::string_find_first(m_path, ':') != pfc_infinite || (m_path.length() > 1 && m_path.get_ptr()[0] == '\\' && m_path.get_ptr()[1] == '\\');
	bool b_relative_to_drive = !b_absolute && m_path.get_ptr()[0] == '\\';

	pfc::string8 fb2kexe;
	uGetModuleFileName(NULL, fb2kexe);
	//pfc::string8 fullPath;

	if (b_relative_to_drive)
	{
		t_size index_colon = fb2kexe.find_first(':');
		if (index_colon != pfc_infinite)
			p_out.add_string(fb2kexe.get_ptr(), index_colon + 1);
	}
	else if (!b_absolute)
		p_out << pfc::string_directory(fb2kexe) << "\\";
	p_out += m_path;
}
void toolbar_extension::button::custom_image::write(stream_writer * out, abort_callback & p_abort) const
{
	out->write_string(m_path, p_abort);
	out->write_lendian_t(m_mask_type, p_abort);
	if (m_mask_type == uie::MASK_BITMAP)
	{
		out->write_string(m_mask_path, p_abort);
	}
	else if (m_mask_type == uie::MASK_COLOUR)
		out->write_lendian_t(m_mask_colour, p_abort);
}
void toolbar_extension::button::custom_image::read(t_config_version p_version, stream_reader * reader, abort_callback & p_abort)
{
	pfc::string8 temp;
	reader->read_string(temp, p_abort);
	m_path = temp;
	reader->read_lendian_t(m_mask_type, p_abort);
	if (m_mask_type == uie::MASK_BITMAP)
	{
		reader->read_string(temp, p_abort);
		m_mask_path = temp;
	}
	else if (m_mask_type == uie::MASK_COLOUR)
		reader->read_lendian_t(m_mask_colour, p_abort);
}

void toolbar_extension::button::custom_image::read_from_file(t_config_version p_version, const char * p_base, const char * p_name, stream_reader * p_file, unsigned p_size, abort_callback & p_abort)
{
	//t_filesize p_start = p_file->get_position(p_abort);
	t_filesize read = 0;
	while (/*p_file->get_position(p_abort) - p_start*/ read < p_size /* && !p_file->is_eof(p_abort)*/)
	{
		t_identifier id;
		p_file->read_lendian_t(id, p_abort);
		unsigned size;
		p_file->read_lendian_t(size, p_abort);
		//if (size > p_file->get_size(p_abort) - p_file->get_position(p_abort))
		//	throw exception_io_data();
		pfc::array_t<char> path, maskpath;
		read += 8 + size;
		switch (id)
		{
		case I_BUTTON_MASK_TYPE:
			p_file->read_lendian_t(m_mask_type, p_abort);
			break;
		case I_BUTTON_MASK_COLOUR:
			p_file->read_lendian_t(m_mask_colour, p_abort);
			break;
		case I_CUSTOM_BUTTON_MASK_PATH:
			maskpath.set_size(size);
			p_file->read(maskpath.get_ptr(), maskpath.get_size(), p_abort);
			m_mask_path.set_string(maskpath.get_ptr(), maskpath.get_size());
			break;
		case I_CUSTOM_BUTTON_PATH:
		{
			path.set_size(size);
			p_file->read(path.get_ptr(), path.get_size(), p_abort);
			m_path.set_string(path.get_ptr(), path.get_size());
		}
		break;
		case I_BUTTON_CUSTOM_IMAGE_DATA:
		{
			t_filesize read2 = 0;
			//t_filesize p_start_data = p_file->get_position(p_abort);
			pfc::array_t<char> name;
			pfc::array_t<t_uint8> data;
			while (read2 /*p_file->get_position(p_abort) - p_start_data*/ < size/* && !p_file->is_eof(p_abort)*/)
			{
				DWORD size_data;
				t_identifier id_data;
				p_file->read_lendian_t(id_data, p_abort);
				p_file->read_lendian_t(size_data, p_abort);
				//if (size_data > p_file->get_size(p_abort) - p_file->get_position(p_abort))
				//throw exception_io_data();
				read2 += 8 + size_data;
				switch (id_data)
				{
				case IMAGE_NAME:
					name.set_size(size_data);
					p_file->read(name.get_ptr(), name.get_size(), p_abort);
					break;
				case IMAGE_DATA:
					data.set_size(size_data);
					p_file->read(data.get_ptr(), data.get_size(), p_abort);
					break;
				default:
					if (p_file->skip(size_data, p_abort) != size_data)
						throw exception_io_data_truncation();
					break;
				}
			}
				{
					pfc::string_printf dir1("%s\\images", p_base);
					pfc::string_printf dir2("%s\\images\\%s", p_base, p_name);
					if (!filesystem::g_exists(dir1, p_abort))
						filesystem::g_create_directory(dir1, p_abort);
					if (!filesystem::g_exists(dir2, p_abort))
						filesystem::g_create_directory(dir2, p_abort);

					service_ptr_t<file> p_image;
					pfc::string8 curdir, wname;

					curdir = pfc::string_printf("%s\\images", p_base);

					wname = curdir;
					wname.add_byte('\\');

					wname.add_string(p_name);
					wname.add_byte('\\');

					wname.add_string(name.get_ptr(), name.get_size());
					pfc::string8 name_only = pfc::string_filename(wname);
					pfc::string8 ext = pfc::string_extension(wname).get_ptr();
					unsigned n = 0;

					bool b_write = true;
					{
						bool b_continue = false;
						do
						{
							bool b_exists = filesystem::g_exists(wname, p_abort);
							if (b_exists)
							{
								b_continue = true;
								service_ptr_t<file> p_temp;
								try
								{
									filesystem::g_open(p_temp, wname, filesystem::open_mode_read, p_abort);
									{
										bool b_same = false;
										g_compare_file_with_bytes(p_temp, data, b_same, p_abort);
										if (b_same)
										{
											b_write = false;
											b_continue = false;
										}
									}
								}
								catch (pfc::exception &)
								{
								}
							}
							else b_continue = false;

							if (b_continue && n<100)
								wname = pfc::string_printf("%s\\%s\\%s %02u.%s", curdir.get_ptr(), p_name, name_only.get_ptr(), n, ext.get_ptr()); n++;

						} while (b_continue && n<100);
					}

					if (b_write)
					{
						filesystem::g_open(p_image, wname, filesystem::open_mode_write_new, p_abort);
						p_image->write(data.get_ptr(), data.get_size(), p_abort);
					}
					m_path = wname;
				}
		}
		break;
		case I_BUTTON_CUSTOM_IMAGE_MASK_DATA:
		{
			t_filesize read2 = 0;
			//t_filesize p_start_data = p_file->get_position(p_abort);
			pfc::array_t<char> name;
			pfc::array_t<t_uint8> data;
			while (read2 /*p_file->get_position(p_abort) - p_start_data*/ < size/* && !p_file->is_eof(p_abort)*/)
			{
				DWORD size_data;
				t_identifier id_data;
				p_file->read_lendian_t(id_data, p_abort);
				p_file->read_lendian_t(size_data, p_abort);
				//if (size_data > p_file->get_size(p_abort) - p_file->get_position(p_abort))
				//	throw exception_io_data();
				read2 += 8 + size_data;
				switch (id_data)
				{
				case IMAGE_NAME:
					name.set_size(size_data);
					p_file->read(name.get_ptr(), name.get_size(), p_abort);
					break;
				case IMAGE_DATA:
					data.set_size(size_data);
					p_file->read(data.get_ptr(), data.get_size(), p_abort);
					break;
				default:
					if (p_file->skip(size_data, p_abort) != size_data)
						throw exception_io_data_truncation();
					break;
				}
			}
				{
					pfc::string_printf dir1("%s\\images", p_base);
					pfc::string_printf dir2("%s\\images\\%s", p_base, p_name);
					if (!filesystem::g_exists(dir1, p_abort))
						filesystem::g_create_directory(dir1, p_abort);
					if (!filesystem::g_exists(dir2, p_abort))
						filesystem::g_create_directory(dir2, p_abort);
					service_ptr_t<file> p_image;
					pfc::string8 curdir, wname;

					curdir = pfc::string_printf("%s\\images", p_base);

					wname = curdir;
					wname.add_byte('\\');

					wname.add_string(p_name);
					wname.add_byte('\\');

					wname.add_string(name.get_ptr(), name.get_size());

					pfc::string8 name_only = pfc::string_filename(wname);
					pfc::string8 ext = pfc::string_extension(wname).get_ptr();
					unsigned n = 0;
					bool b_write = true;
					{
						bool b_continue = false;
						do
						{
							bool b_exists = filesystem::g_exists(wname, p_abort);
							if (b_exists)
							{
								b_continue = true;
								service_ptr_t<file> p_temp;
								try
								{
									filesystem::g_open(p_temp, wname, filesystem::open_mode_read, p_abort);
									{
										bool b_same = false;
										g_compare_file_with_bytes(p_temp, data, b_same, p_abort);
										if (b_same)
										{
											b_write = false;
											b_continue = false;
										}
									}
								}
								catch (pfc::exception &)
								{
								}
							}
							else b_continue = false;

							if (b_continue && n<100)
								wname = pfc::string_printf("%s\\%s\\%s %02u.%s", curdir.get_ptr(), p_name, name_only.get_ptr(), n, ext.get_ptr()); n++;

						} while (b_continue && n<100);
					}


					if (b_write)
					{
						filesystem::g_open(p_image, wname, filesystem::open_mode_write_new, p_abort);
						p_image->write(data.get_ptr(), data.get_size(), p_abort);
					}
					m_mask_path = wname;
				}
		}
		break;
		default:
			if (p_file->skip(size, p_abort) != size)
				throw exception_io_data_truncation();
			break;
		}
	}
}


void toolbar_extension::button::custom_image::write_to_file(stream_writer &p_file, bool b_paths, abort_callback & p_abort)
{
	p_file.write_lendian_t(I_BUTTON_MASK_TYPE, p_abort);
	p_file.write_lendian_t(sizeof(m_mask_type), p_abort);
	p_file.write_lendian_t(m_mask_type, p_abort);

	if (b_paths)
	{
		p_file.write_lendian_t(I_CUSTOM_BUTTON_PATH, p_abort);
		p_file.write_lendian_t(m_path.length(), p_abort);
		p_file.write(m_path.get_ptr(), m_path.length(), p_abort);
	}
	else
	{
		pfc::string8 realPath, canPath;
		try{
			p_file.write_lendian_t(I_BUTTON_CUSTOM_IMAGE_DATA, p_abort);

			{
				service_ptr_t<file> p_image;
				get_path(realPath);
				filesystem::g_get_canonical_path(realPath, canPath);
				filesystem::g_open(p_image, canPath, filesystem::open_mode_read, p_abort);

				pfc::string_filename_ext name(m_path);

				t_filesize imagesize = p_image->get_size(p_abort);

				if (imagesize >= MAXLONG)
					throw exception_io_device_full();

				unsigned size = (unsigned)imagesize + name.length() + 4 * sizeof(t_uint32);
				p_file.write_lendian_t(size, p_abort);

				p_file.write_lendian_t(IMAGE_NAME, p_abort);
				p_file.write_lendian_t(name.length(), p_abort);
				p_file.write(name.get_ptr(), name.length(), p_abort);

				p_file.write_lendian_t(IMAGE_DATA, p_abort);
				p_file.write_lendian_t((unsigned)imagesize, p_abort);
				pfc::array_t<t_uint8> temp;
				temp.set_size((unsigned)imagesize);
				p_image->read(temp.get_ptr(), temp.get_size(), p_abort);
				p_file.write(temp.get_ptr(), temp.get_size(), p_abort);
			}
		}
		catch (const pfc::exception & err)
		{
			pfc::string_formatter formatter;
			throw pfc::exception(formatter << "Error reading file \"" << realPath << "\" : " << err.what());
		}
	}
	if (m_mask_type == uie::MASK_BITMAP)
	{
		if (b_paths)
		{
			p_file.write_lendian_t(I_CUSTOM_BUTTON_MASK_PATH, p_abort);
			p_file.write_lendian_t(m_mask_path.length(), p_abort);
			p_file.write(m_mask_path.get_ptr(), m_mask_path.length(), p_abort);
		}
		else
		{
			try {
				p_file.write_lendian_t(I_BUTTON_CUSTOM_IMAGE_MASK_DATA, p_abort);

				service_ptr_t<file> p_image;
				filesystem::g_open(p_image, m_mask_path, filesystem::open_mode_read, p_abort);

				pfc::string_filename_ext name(m_mask_path);

				t_filesize imagesize = p_image->get_size(p_abort);

				if (imagesize >= MAXLONG)
					throw exception_io_device_full();

				unsigned size = (unsigned)imagesize + name.length() + sizeof(IMAGE_NAME) + sizeof(IMAGE_DATA);
				p_file.write_lendian_t(size, p_abort);

				p_file.write_lendian_t(IMAGE_NAME, p_abort);
				p_file.write_lendian_t(name.length(), p_abort);
				p_file.write(name.get_ptr(), name.length(), p_abort);

				p_file.write_lendian_t(IMAGE_DATA, p_abort);
				p_file.write_lendian_t((unsigned)imagesize, p_abort);
				pfc::array_t<t_uint8> temp;
				temp.set_size((unsigned)imagesize);
				p_image->read(temp.get_ptr(), temp.get_size(), p_abort);
				p_file.write(temp.get_ptr(), temp.get_size(), p_abort);
			}
			catch (const pfc::exception & err)
			{
				pfc::string_formatter formatter;
				throw pfc::exception(formatter << "Error reading file \"" << m_mask_path << "\" : " << err.what());
			}
		}
	}

	if (m_mask_type == uie::MASK_COLOUR)
	{
		p_file.write_lendian_t(I_BUTTON_MASK_COLOUR, p_abort);
		p_file.write_lendian_t(sizeof(m_mask_colour), p_abort);
		p_file.write_lendian_t(m_mask_colour, p_abort);
	}
}


